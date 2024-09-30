from dataset import SimpleDataset
import torch
import torch.nn as nn
from torch.utils.data import DataLoader, Subset
from sklearn.model_selection import KFold
from tqdm import tqdm
import matplotlib.pyplot as plt
import random
import segmentation_models_pytorch as smp
from torchvision.transforms import transforms
import argparse
import os
import shutil
from pathlib import Path
import logging

# Importing custom module
import sys
dir_path = os.path.dirname(os.path.realpath(__file__))
build_dir = os.path.join(dir_path, "Library/Connectivity/build/lib.linux-x86_64-3.10")
sys.path.insert(0, build_dir)
import connectivity

# Global constants
SAVE_PATH = ""
TRANSFORM_RESIZE = None

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)


def visualize_results(dataset, model, name, device):
    """Visualize the input, target, and model output."""
    model.eval()

    idx = random.randint(0, len(dataset) - 1)
    data, target, _ = dataset[idx]
    data = data.unsqueeze(0).to(device)

    with torch.no_grad():
        output = model(data)

    output = torch.sigmoid(output)
    output = (output > 0.5).float()

    data_np = (TRANSFORM_RESIZE(data).cpu().squeeze(0).permute(1, 2, 0).numpy() * 255).astype(int)
    target_np = (TRANSFORM_RESIZE(target).cpu().permute(1, 2, 0).numpy() * 255).astype(int)
    output_np = (TRANSFORM_RESIZE(output).cpu().squeeze(0).permute(1, 2, 0).numpy() * 255).astype(int)

    fig, axes = plt.subplots(1, 3, figsize=(12, 6))

    for ax, img, title in zip(axes, [data_np, target_np, output_np], ['Input', 'Ground Truth', 'Prediction']):
        ax.imshow(img, cmap='viridis')
        ax.set_title(title)
        ax.axis('off')

    plt.tight_layout()
    plt.savefig(SAVE_PATH / f'{name}.png', dpi=200)
    plt.close()


def criterion(output, target):
    """Combined BCE and Dice loss."""
    bce_criterion = smp.losses.SoftBCEWithLogitsLoss()
    dice_criterion = smp.losses.DiceLoss('multilabel')
    return 0.5 * bce_criterion(output, target) + 0.5 * dice_criterion(output, target)


def train_model(model, train_loader, val_loader, device, num_epochs, fold):
    """Train and validate the model for a given fold."""
    optimizer = torch.optim.AdamW(model.parameters(), lr=1e-4)
    scheduler = torch.optim.lr_scheduler.ExponentialLR(optimizer, gamma=0.985, verbose=True)
    best_val_loss = float('inf')
    best_model_path = SAVE_PATH / f'best_model_fold_{fold + 1}.pth'

    for epoch in range(num_epochs):
        model.train()
        train_loss = 0.0

        for data, targets, _ in tqdm(train_loader, desc=f"Epoch {epoch + 1}/{num_epochs} - Training", ncols=80):
            data, targets = data.to(device), targets.to(device)
            optimizer.zero_grad()
            outputs = model(data)
            loss = criterion(outputs, targets)
            loss.backward()
            optimizer.step()
            train_loss += loss.item() * data.size(0)

        train_loss /= len(train_loader.dataset)

        model.eval()

        val_loss = 0.0
        connectivity_loss = 0.0

        with torch.no_grad():
            for data, targets, nodes in tqdm(val_loader, desc=f"Epoch {epoch + 1}/{num_epochs} - Validation", ncols=80):
                data, targets, nodes = data.to(device), targets.to(device), nodes.numpy()
                outputs = model(data)

                loss = smp.losses.DiceLoss('multilabel')(outputs, targets)
                val_loss += loss.item() * data.size(0)

                outputs_np = (TRANSFORM_RESIZE((torch.sigmoid(outputs.detach()) > 0.5).float()).cpu().numpy()).astype(int)
                connectivity_loss += connectivity.single_net_check_connectivity(outputs_np, nodes)

        val_loss /= len(val_loader.dataset)
        connectivity_loss /= len(val_loader.dataset)

        logger.info(f'\n\nEpoch {epoch + 1}/{num_epochs}, Train Loss: {train_loss:.4f}, Validation Dice Loss: {val_loss:.4f}, Connectivity score: {connectivity_loss:.4f}')

        if connectivity_loss > best_val_loss:
            logger.info(f'Validation loss decreased ({best_val_loss:.6f} --> {connectivity_loss:.6f}). Saving model...')

            torch.save(model.state_dict(), best_model_path)
            best_val_loss = connectivity_loss

        visualize_results(val_loader.dataset, model, f"validation_example_fold_{fold + 1}_epoch_{epoch + 1}", device)
        visualize_results(train_loader.dataset, model, f"train_example_fold_{fold + 1}_epoch_{epoch + 1}", device)

        scheduler.step()


def reset_weights(m):
    """Reset model weights to avoid weight leakage between folds."""
    if isinstance(m, (nn.Conv2d, nn.Linear)):
        m.reset_parameters()


def k_fold_training(dataset, model_class, device, k, num_epochs):
    """Perform K-Fold training."""
    kfold = KFold(n_splits=k, shuffle=True, random_state=42)

    for fold, (train_idx, val_idx) in enumerate(kfold.split(dataset)):
        logger.info(f"\nFold {fold + 1}/{k}\n")

        model = model_class().to(device)
        model.apply(reset_weights)

        train_subset = Subset(dataset, train_idx)
        val_subset = Subset(dataset, val_idx)

        train_loader = DataLoader(train_subset, batch_size=64, shuffle=True)
        val_loader = DataLoader(val_subset, batch_size=64, shuffle=False)

        train_model(model, train_loader, val_loader, device, num_epochs, fold)


def get_model():
    """Return a U-Net model."""
    return smp.Unet(encoder_name="efficientnet-b3", encoder_weights="imagenet", in_channels=1, classes=1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process parameters.")
    parser.add_argument('--input_size', type=int, help='Input size')
    parser.add_argument('--scale_size', type=int, help='Scale size')
    parser.add_argument('--assets_dir', type=str, help='Assets directory')
    parser.add_argument('--save_path', type=str, help='File path where the output image and model should be saved')

    args = parser.parse_args()

    input_size = args.input_size
    SAVE_PATH = Path(args.save_path)
    TRANSFORM_RESIZE = transforms.Compose([transforms.Resize((input_size, input_size), interpolation=transforms.InterpolationMode.NEAREST)])

    if SAVE_PATH.exists():
        shutil.rmtree(SAVE_PATH)

    SAVE_PATH.mkdir(parents=True)

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    dataset = SimpleDataset(args.assets_dir, (args.scale_size, args.scale_size), 1.0)

    k_fold_training(dataset, get_model, device, k=5, num_epochs=50)
