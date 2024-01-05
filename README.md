# Reinforcement Learning Grid Pathfinding

## Project Description
This project focuses on training a reinforcement learning agent to find the optimal path between two points in a grid environment with obstacles using Stable Baselines 3. The project utilizes custom environments and callbacks for effective training and monitoring.

## Getting Started

### Prerequisites
- Python >=3.5
- Pip (Python package manager)

### Installation
1. **Clone the repository:**
   ```bash
   git clone [URL-of-your-repo]
   cd rl-grid-pathfinding
   ```

2. **Install required packages:**
   ```bash
   pip install -r requirements.txt
   ```

### Project Structure
- `envs/`: Custom environment definitions.
- `utils/`: Utility scripts and functions.
- `train.py`: Scripts for training models.
- `requirements.txt`: List of Python package dependencies.

## Usage

### Training the Agent
To train the agent, run the script `train.py`. You can specify various parameters like grid size and total timesteps:

```bash
python3 train.py --num_envs 16 --bot_grid_size 16 --top_grid_size 256 --total_timesteps 5000000
```

## Contributing
Contributions to this project are welcome. Please ensure that your code adheres to the project's coding standards and submit a pull request for review.

## License
This project is licensed under the [MIT License](LICENSE).