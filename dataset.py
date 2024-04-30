import torch
from torch.utils.data import Dataset
from torchvision.transforms import transforms
import os
from tqdm import tqdm


class MatrixDataset(Dataset):
    def __init__(self, folder_path):
        super(MatrixDataset, self).__init__()
        self.data_files = []
        self.target_files = []
        self.terminals_files = []
        self.transform = transforms.Compose([transforms.Resize(224, interpolation=transforms.InterpolationMode.NEAREST), transforms.Pad(16)])

        self.processed_folder = os.path.join(folder_path, 'processed')
        os.makedirs(self.processed_folder, exist_ok=True)

        for filename in tqdm(os.listdir(folder_path), desc=f"Processing dataset", bar_format='{desc}: {percentage:3.0f}%|{bar}| {n_fmt}/{total_fmt} [{elapsed}<{remaining}, {rate_fmt}{postfix}]', ncols=80):
            if not filename.endswith('.pt'):
                file_path = os.path.join(folder_path, filename)

                if (os.path.isfile(file_path)):
                    data_path, target_path, terminals_path = self.process_file(file_path, filename)

                    self.data_files.append(data_path)
                    self.target_files.append(target_path)
                    self.terminals_files.append(terminals_path)

    def process_file(self, filepath, filename):
        data_file_path = os.path.join(self.processed_folder, f'{filename}_data.pt')
        target_file_path = os.path.join(self.processed_folder, f'{filename}_target.pt')
        terminals_file_path = os.path.join(self.processed_folder, f'{filename}_terminals.pt')

        if (os.path.exists(data_file_path) and os.path.exists(target_file_path)):
            return data_file_path, target_file_path, terminals_file_path

        with open(filepath, 'r') as file:
            content = file.readlines()

        shape_line = content[0].strip()
        shape = tuple(map(int, shape_line.split(' ')[1:]))

        terminals_line = content[1].strip()
        terminals = list(map(int, terminals_line.split(' ')[1:]))
        terminals = [len(terminals)] + terminals

        if (terminals[0] < 20):
            terminals += [0 for i in range(20 - terminals[0])]
        elif (terminals[0] > 20):
            print(terminals)

        terminals_tensor = torch.tensor(terminals, dtype=torch.long)

        i = 2
        data_tensor = None
        target_tensor = None

        while i < len(content):
            if content[i].strip() == "INPUT:":
                input_matrix = []
                i += 1

                while i < len(content) and content[i].strip() != "OUTPUT:":
                    row = list(map(float, content[i].strip().split()))
                    input_matrix.append(row)
                    i += 1

                data_tensor = self.transform(torch.tensor(input_matrix, dtype=torch.float32).view(shape).unsqueeze(0) / 4.0)

            elif content[i].strip() == "OUTPUT:":
                output_matrix = []
                i += 1

                while i < len(content) and content[i].strip() not in ["INPUT:", "SHAPE:"]:
                    row = list(map(float, content[i].strip().split()))
                    output_matrix.append(row)
                    i += 1

                target_tensor = self.transform(torch.tensor(output_matrix, dtype=torch.float32).view(shape).unsqueeze(0))

        torch.save(terminals_tensor, terminals_file_path)
        torch.save(data_tensor, data_file_path)
        torch.save(target_tensor, target_file_path)

        return data_file_path, target_file_path, terminals_file_path

    def __len__(self):
        return len(self.data_files)

    def __getitem__(self, idx):
        data_tensor = torch.load(self.data_files[idx])
        target_tensor = torch.load(self.target_files[idx])
        terminals_tensor = torch.load(self.terminals_files[idx])
        return data_tensor, target_tensor, terminals_tensor
