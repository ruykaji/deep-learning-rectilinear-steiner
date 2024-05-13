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

from dataset import MatrixDataset
import connectivity_module as cm

SAVE_PATH = ""
TRANSFORM_NORMAL = transforms.Compose([])


def visualize_results(val_dataset, model, name, device):
    model.eval()

    idx = random.randint(0, len(val_dataset) - 1)
    data, target, _ = val_dataset[idx]
    data = data.unsqueeze(0).to(device)
    target = target.to(device)

    with torch.no_grad():
        output = model(data)

    output = torch.sigmoid(output)
    output = torch.where(output > 0.25, 1.0, output * 0.0)

    output = TRANSFORM_NORMAL(output).squeeze().cpu().numpy()
    target = TRANSFORM_NORMAL(target).squeeze().cpu().numpy()
    data = TRANSFORM_NORMAL(data).squeeze().cpu().numpy()

    plt.figure(figsize=(12, 6))
    plt.subplot(1, 3, 1)
    plt.imshow(data, cmap='gray')
    plt.title('Input')
    plt.subplot(1, 3, 2)
    plt.imshow(target, cmap='gray')
    plt.title('Ground Truth')
    plt.subplot(1, 3, 3)
    plt.imshow(output, cmap='gray')
    plt.title('Prediction')
    plt.savefig(os.path.join(SAVE_PATH, f'{name}.png'), dpi=400)
    plt.close()


def train_model(model, train_loader, val_loader, device, num_epochs, fold):
    bce_criterion = nn.BCEWithLogitsLoss()
    dice_criterion = smp.losses.DiceLoss('multilabel')
    optimizer = torch.optim.AdamW(model.parameters(), lr=1e-4)
    scheduler = torch.optim.lr_scheduler.ExponentialLR(optimizer, gamma=0.98, verbose=True)
    best_val_loss = 0
    best_model_path = os.path.join(SAVE_PATH, f'best_model_fold_{fold+1}.pth')

    def criterion(output, target):
        return bce_criterion(output, target) * 0.5 + dice_criterion(output, target) * 0.5

    with open(os.path.join(SAVE_PATH, "train.log"), 'w') as logger:
        for epoch in range(num_epochs):
            model.train()
            train_loss = 0.0

            for data, targets, _ in tqdm(train_loader, desc=f"Epoch {epoch+1}/{num_epochs} - Training", bar_format='{desc}: {percentage:3.0f}%|{bar}| {n_fmt}/{total_fmt} [{elapsed}<{remaining}, {rate_fmt}{postfix}]', ncols=80):
                data, targets = data.to(device), targets.to(device)
                optimizer.zero_grad()
                outputs = model(data)
                loss = criterion(outputs, targets)
                loss.backward()
                optimizer.step()
                train_loss += loss.item() * data.size(0)

            model.eval()
            dice_loss = 0.0
            connectivity_score = 0.0

            with torch.no_grad():
                for data, targets, terminals in tqdm(val_loader, desc=f"Epoch {epoch+1}/{num_epochs} - Validation", bar_format='{desc}: {percentage:3.0f}%|{bar}| {n_fmt}/{total_fmt} [{elapsed}<{remaining}, {rate_fmt}{postfix}]', ncols=80):
                    data, targets = data.to(device), targets.to(device)

                    outputs = TRANSFORM_NORMAL(model(data)).squeeze()

                    loss = dice_criterion(outputs.clone(), TRANSFORM_NORMAL(targets))
                    dice_loss += loss.item() * data.size(0)

                    outputs = torch.sigmoid(outputs)
                    outputs = torch.where(outputs > 0.25, 1.0, outputs * 0.0).cpu().numpy()
                    connectivity_score += sum(cm.are_batches_terminals_connected(outputs, terminals.tolist()))

            train_loss /= len(train_loader.dataset)
            dice_loss /= len(val_loader.dataset)
            connectivity_score /= len(val_loader.dataset)

            print(f'\nEpoch {epoch+1}/{num_epochs}, Train Loss: {train_loss:.4f}, Dice Loss: {dice_loss:.4f}, Connectivity score: {connectivity_score:.4f}')
            logger.write(f'\nEpoch {epoch+1}/{num_epochs}, Train Loss: {train_loss:.4f}, Dice Loss: {dice_loss:.4f}, Connectivity score: {connectivity_score:.4f}')

            if connectivity_score > best_val_loss:
                print('Validation loss decreased ({:.6f} --> {:.6f}). Saving model...'.format(best_val_loss, connectivity_score))
                logger.write('Validation loss decreased ({:.6f} --> {:.6f}). Saving model...'.format(best_val_loss, connectivity_score))

                torch.save(model.state_dict(), best_model_path)
                best_val_loss = connectivity_score

                visualize_results(train_loader.dataset, model, "validation_example", device)
                visualize_results(val_loader.dataset, model, "train_example", device)

            scheduler.step()
            print()
            logger.write('\n')

        logger.close()


def reset_weights(m):
    if isinstance(m, nn.Conv2d) or isinstance(m, nn.Linear):
        m.reset_parameters()


def k_fold_training(dataset, model, device, k, num_epochs):
    kfold = KFold(n_splits=k, shuffle=True, random_state=42)

    for fold, (train_idx, val_idx) in enumerate(kfold.split(dataset)):
        print(f"\nFold {fold+1}/{k}\n")

        fold_model = model.to(device)
        fold_model.apply(reset_weights)

        train_subset = Subset(dataset, train_idx)
        val_subset = Subset(dataset, val_idx)

        train_loader = DataLoader(train_subset, batch_size=64, shuffle=True)
        val_loader = DataLoader(val_subset, batch_size=64, shuffle=False)

        train_model(fold_model, train_loader, val_loader, device, num_epochs, fold)
        return


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process parameters.")
    parser.add_argument('--size', type=str, help='Input size')
    parser.add_argument('--save_path', type=str, help='File path where the output image and model should be saved')

    args = parser.parse_args()

    SAVE_PATH = args.save_path
    TRANSFORM_NORMAL = transforms.Compose([transforms.Resize(int(args.size), interpolation=transforms.InterpolationMode.NEAREST)])

    if (not os.path.exists(SAVE_PATH)):
        os.makedirs(SAVE_PATH)

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    model = smp.Unet(encoder_name="efficientnet-b3", encoder_weights="imagenet", in_channels=1, classes=1).to(device)
    dataset = MatrixDataset('./assets')

    k_fold_training(dataset, model, device, k=5, num_epochs=20)
