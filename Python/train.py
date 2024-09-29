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

SAVE_PATH = ""
TRANSFORM_RESIZE = None


def visualize_results(dataset, model, name, device):
    model.eval()

    idx = random.randint(0, len(dataset) - 1)
    data, target = dataset[idx]
    data = data.unsqueeze(0).to(device)

    with torch.no_grad():
        output = model(data)

    output = torch.sigmoid(output)
    output = (output > 0.5).float()

    data_np = (TRANSFORM_RESIZE(data).cpu().squeeze(0).permute(1, 2, 0).numpy() * 255).astype(int)
    target_np = (TRANSFORM_RESIZE(target).cpu().permute(1, 2, 0).numpy() * 255).astype(int)
    output_np = (TRANSFORM_RESIZE(output).cpu().squeeze(0).permute(1, 2, 0).numpy() * 255).astype(int)

    fig, axes = plt.subplots(1, 3, figsize=(12, 6))

    axes[0].imshow(data_np, cmap='viridis')
    axes[0].set_title('Input')
    axes[0].axis('off')

    axes[1].imshow(target_np, cmap='viridis')
    axes[1].set_title('Ground Truth')
    axes[1].axis('off')

    axes[2].imshow(output_np, cmap='viridis')
    axes[2].set_title('Prediction')
    axes[2].axis('off')

    plt.tight_layout()
    plt.savefig(os.path.join(SAVE_PATH, f'{name}.png'), dpi=200)
    plt.close()


def train_model(model, train_loader, val_loader, device, num_epochs, fold, logger):
    bce_criterion = smp.losses.SoftBCEWithLogitsLoss()
    dice_criterion = smp.losses.DiceLoss('multilabel')
    optimizer = torch.optim.AdamW(model.parameters(), lr=1e-4)
    scheduler = torch.optim.lr_scheduler.ExponentialLR(optimizer, gamma=0.985, verbose=True)
    best_val_loss = float('inf')
    best_model_path = os.path.join(SAVE_PATH, f'best_model_fold_{fold+1}.pth')

    def criterion(output, target):
        return 0.5 * bce_criterion(output, target) + 0.5 * dice_criterion(output, target)

    for epoch in range(num_epochs):
        model.train()
        train_loss = 0.0

        for data, targets in tqdm(train_loader, desc=f"Epoch {epoch+1}/{num_epochs} - Training", ncols=80):
            data, targets = data.to(device), targets.to(device)
            optimizer.zero_grad()
            outputs = model(data)
            loss = criterion(outputs, targets)
            loss.backward()
            optimizer.step()
            train_loss += loss.item() * data.size(0)

        train_loss /= len(train_loader.dataset)

        model.eval()
        dice_loss = 0.0

        with torch.no_grad():
            for data, targets in tqdm(val_loader, desc=f"Epoch {epoch+1}/{num_epochs} - Validation", ncols=80):
                data, targets = data.to(device), targets.to(device)
                outputs = model(data)
                loss = dice_criterion(outputs, targets)
                dice_loss += loss.item() * data.size(0)

        dice_loss /= len(val_loader.dataset)

        print(f'\nEpoch {epoch+1}/{num_epochs}, Train Loss: {train_loss:.4f}, Dice Loss: {dice_loss:.4f}')
        logger.write(f'\nEpoch {epoch+1}/{num_epochs}, Train Loss: {train_loss:.4f}, Dice Loss: {dice_loss:.4f}\n')

        if dice_loss < best_val_loss:
            print('Validation loss decreased ({:.6f} --> {:.6f}). Saving model...'.format(best_val_loss, dice_loss))
            logger.write('Validation loss decreased ({:.6f} --> {:.6f}). Saving model...\n'.format(best_val_loss, dice_loss))

            torch.save(model.state_dict(), best_model_path)
            best_val_loss = dice_loss

        visualize_results(val_loader.dataset, model, f"validation_example_fold_{fold+1}_epoch_{epoch+1}", device)
        visualize_results(train_loader.dataset, model, f"train_example_fold_{fold+1}_epoch_{epoch+1}", device)

        scheduler.step()

        print()
        logger.write('\n')


def reset_weights(m):
    if isinstance(m, nn.Conv2d) or isinstance(m, nn.Linear):
        m.reset_parameters()


def k_fold_training(dataset, model_class, device, k, num_epochs):
    kfold = KFold(n_splits=k, shuffle=True, random_state=42)

    with open(os.path.join(SAVE_PATH, "train.log"), 'w') as logger:
        for fold, (train_idx, val_idx) in enumerate(kfold.split(dataset)):
            print(f"\nFold {fold+1}/{k}\n")
            logger.write(f"\nFold {fold+1}/{k}\n")

            # Instantiate a new model for each fold
            model = model_class().to(device)
            model.apply(reset_weights)

            train_subset = Subset(dataset, train_idx)
            val_subset = Subset(dataset, val_idx)

            train_loader = DataLoader(train_subset, batch_size=64, shuffle=True)
            val_loader = DataLoader(val_subset, batch_size=64, shuffle=False)

            train_model(model, train_loader, val_loader, device, num_epochs, fold, logger)


def get_model():
    return smp.Unet(encoder_name="efficientnet-b3", encoder_weights="imagenet", in_channels=1, classes=1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process parameters.")
    parser.add_argument('--input_size', type=int, help='Input size')
    parser.add_argument('--scale_size', type=int, help='Scale size')
    parser.add_argument('--source_dir', type=str, help='Source directory')
    parser.add_argument('--target_dir', type=str, help='Target directory')
    parser.add_argument('--save_path', type=str, help='File path where the output image and model should be saved')

    args = parser.parse_args()

    input_size = args.input_size

    SAVE_PATH = args.save_path
    TRANSFORM_RESIZE = transforms.Compose([transforms.Resize((input_size, input_size), interpolation=transforms.InterpolationMode.NEAREST)])

    if os.path.exists(SAVE_PATH):
        shutil.rmtree(SAVE_PATH)

    os.makedirs(SAVE_PATH)

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    dataset = SimpleDataset(args.source_dir, args.target_dir, (args.scale_size, args.scale_size), 1.0, device)

    k_fold_training(dataset, get_model, device, k=5, num_epochs=50)
