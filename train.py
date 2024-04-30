import torch
import torch.nn as nn
from torch.utils.data import DataLoader, Subset
from sklearn.model_selection import KFold
from tqdm import tqdm
import matplotlib.pyplot as plt
import random
import segmentation_models_pytorch as smp
from torchvision.transforms import transforms

from dataset import MatrixDataset
import connectivity_module as cm


def visualize_results(val_dataset, model, name, device):
    model.eval()

    idx = random.randint(0, len(val_dataset) - 1)
    data, target, _ = val_dataset[idx]
    data = data.unsqueeze(0).to(device)
    target = target.to(device)

    with torch.no_grad():
        output = model(data)

    output = torch.sigmoid(output)
    output = torch.where(output > 0.4, 1.0, output * 0.0)

    transform = transforms.Compose([transforms.CenterCrop(224), transforms.Resize(32, interpolation=transforms.InterpolationMode.NEAREST)])

    output = transform(output).squeeze().cpu().numpy()
    target = transform(target).squeeze().cpu().numpy()
    data = transform(data).squeeze().cpu().numpy()

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
    plt.savefig(f'{name}.png', dpi=400)
    plt.close()


def train_model(model, train_loader, val_loader, device, num_epochs, fold):
    criterion = nn.BCEWithLogitsLoss()
    dice_loss = smp.losses.DiceLoss('multilabel')
    optimizer = torch.optim.AdamW(model.parameters(), lr=1e-4)
    scheduler = torch.optim.lr_scheduler.ExponentialLR(optimizer, gamma=0.98, verbose=True)
    best_val_loss = float('inf')
    best_model_path = f'best_model_fold_{fold+1}.pth'

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
        val_loss = 0.0
        new_loss = 0.0

        with torch.no_grad():
            transform = transforms.Compose([transforms.CenterCrop(224), transforms.Resize(32, interpolation=transforms.InterpolationMode.NEAREST)])

            for data, targets, terminals in tqdm(val_loader, desc=f"Epoch {epoch+1}/{num_epochs} - Validation", bar_format='{desc}: {percentage:3.0f}%|{bar}| {n_fmt}/{total_fmt} [{elapsed}<{remaining}, {rate_fmt}{postfix}]', ncols=80):
                data, targets = data.to(device), targets.to(device)

                outputs = transform(model(data)).squeeze()

                loss = dice_loss(outputs.clone(), transform(targets))
                val_loss += loss.item() * data.size(0)

                outputs = torch.sigmoid(outputs)
                outputs = torch.where(outputs > 0.4, 1.0, outputs * 0.0).cpu().numpy()
                new_loss += sum(cm.are_batches_terminals_connected(outputs, terminals.tolist()))

        train_loss /= len(train_loader.dataset)
        val_loss /= len(val_loader.dataset)
        new_loss /= len(val_loader.dataset)

        print(f'\nEpoch {epoch+1}/{num_epochs}, Train Loss: {train_loss:.4f}, Val Loss: {val_loss:.4f}, New Loss: {new_loss:.4f}')

        if val_loss < best_val_loss:
            print('Validation loss decreased ({:.6f} --> {:.6f}). Saving model...'.format(best_val_loss, val_loss))
            torch.save(model.state_dict(), best_model_path)
            best_val_loss = val_loss

            visualize_results(train_loader.dataset, model, "validation_example", device)
            visualize_results(val_loader.dataset, model, "train_example", device)

        scheduler.step()
        print()


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


if __name__ == "__main__":
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    model = smp.Unet(encoder_name="efficientnet-b3", encoder_weights="imagenet", in_channels=1, classes=1).to(device)
    dataset = MatrixDataset('./assets')

    # transform = transforms.Compose([transforms.CenterCrop(224), transforms.Resize(32, interpolation=transforms.InterpolationMode.NEAREST)])

    # for data, target, terminals in dataset:
    #     data = data.unsqueeze(0).to(device)
    #     target = transform(target.to(device))

    #     print(cm.are_batches_terminals_connected(target.cpu().numpy(), terminals.unsqueeze(0).cpu().tolist()))

    #     target = target.squeeze().cpu().numpy()
    #     data = data.squeeze().cpu().numpy()

    #     plt.figure(figsize=(12, 6))
    #     plt.subplot(1, 3, 1)
    #     plt.imshow(data, cmap='gray')
    #     plt.title('Input')
    #     plt.subplot(1, 3, 2)
    #     plt.imshow(target, cmap='gray')
    #     plt.title('Ground Truth')
    #     plt.show()
    #     plt.close()

    k_fold_training(dataset, model, device, k=5, num_epochs=50)
