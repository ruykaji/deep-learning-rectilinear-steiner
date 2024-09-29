import os
import numpy as np
from torch.utils.data import Dataset
from torchvision import transforms
import random
import torch
from tqdm import tqdm


class SimpleDataset(Dataset):
    def __init__(self, source_dir, target_dir, image_size, preload_percentage=0.0, device=torch.device('cpu')):
        """
        Args:
            source_dir (str): Path to the source npy files directory.
            target_dir (str): Path to the target npy files directory.
            image_size (tuple): Desired image size as (width, height).
            preload_percentage (float): Percentage of data to preload into memory (0.0 to 1.0).
            device (torch.device): Device to use.
        """

        self.source_dir = source_dir
        self.target_dir = target_dir
        self.image_size = image_size
        self.preload_percentage = preload_percentage
        self.device = device

        self.source_files = sorted(os.listdir(source_dir))
        self.target_files = sorted(os.listdir(target_dir))

        self.transform = transforms.Compose([transforms.Resize(size=image_size, interpolation=transforms.InterpolationMode.NEAREST)])

        self.max_pixel_value = 5

        self.preloaded_data = {}
        self._preload_data()

    def _preload_data(self):
        """Preloads a percentage of the dataset into memory."""
        num_preload = int(self.preload_percentage * len(self.source_files))
        preload_indices = random.sample(range(len(self.source_files)), num_preload)

        for idx in tqdm(preload_indices, desc="Preloading training data", bar_format='{desc}: {percentage:3.0f}%|{bar}| {n_fmt}/{total_fmt} [{elapsed}<{remaining}, {rate_fmt}{postfix}]', ncols=80):
            source_npy_path = os.path.join(self.source_dir, self.source_files[idx])
            source_data = np.load(source_npy_path)
            source_tensor = self.transform(torch.from_numpy(source_data)) / self.max_pixel_value
            source_tensor.to(self.device)

            target_npy_path = os.path.join(self.target_dir, self.target_files[idx])
            target_data = np.load(target_npy_path)
            target_tensor = self.transform(torch.from_numpy(target_data)).type(torch.float32)
            target_tensor.to(self.device)

            self.preloaded_data[idx] = (source_tensor, target_tensor)

    def __len__(self):
        """Returns the total number of npy files in the dataset."""
        return len(self.source_files)

    def __getitem__(self, idx):
        """
        Args:
            idx (int): Index of the data to load.

        Returns:
            Tuple (source_data, target_data): A tuple of the source and target data tensors.
        """

        if idx in self.preloaded_data:
            return self.preloaded_data[idx]

        source_npy_path = os.path.join(self.source_dir, self.source_files[idx])
        source_data = np.load(source_npy_path)
        source_tensor = self.transform(torch.from_numpy(source_data)) / self.max_pixel_value
        source_tensor.to(self.device)

        target_npy_path = os.path.join(self.target_dir, self.target_files[idx])
        target_data = np.load(target_npy_path)
        target_tensor = self.transform(torch.from_numpy(target_data)).type(torch.float32)
        target_tensor.to(self.device)

        return source_tensor, target_tensor