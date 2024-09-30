# Deep Learning Rectilinear Steiner Project

## Project Description
This project aims to solve the Rectilinear Steiner Tree Problem (RSTP) using deep learning.

The project includes:
- A **C++ Sample Generator** for creating training datasets.
- A **Deep Learning Model** implemented in Python using `segmentation-models-pytorch`.

## Prerequisites
- **Python**: >= 3.8.
- **GCC**: >= 12.3.0.
- **CMake**: >= 3.19.0.
- **Docker** (Recommended): To easily set up the environment and dependencies.

## Installation

### 1. Clone the Repository
```bash
git clone https://github.com/ruykaji/deep-learning-rectilinear-steiner.git
cd deep-learning-rectilinear-steiner
```

### 2. Building
#### Manual building
If have installed all necessary utils, then you can just run build script.
```bash
sh Scripts/build.sh
```

## Usage

### 1. Generate dataset
Generator have its own config file `Results/Program/sample_generator_config.ini` that you can edit how you want.
After you done with configuring just run the generation script.
```bash
sh Scripts/generate.sh
```

### 2. Train
Run the train script.
```bash
sh Scripts/train.sh
```