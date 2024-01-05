__all__ = ["BaseGridEnv"]

from typing import List, Any
import cv2
import gymnasium as gym
import numpy as np
import math


class BaseGridEnv(gym.Env):
    """
    Base grid environment.

    :param grid_size: Size of the grid.
    :param grid_step: Step size between each col and row in grid.
    """

    def __init__(self, grid_size: int = 16, grid_step: int = 2):
        super().__init__()

        self.render_mode = "human"
    
        self.grid_size = grid_size
        self.grid_step = grid_step

        # Define time-step counter and threshold time-step value.
        self.timesteps: int = 0
        self.timesteps_thr: int = int(grid_size * grid_size / math.sqrt(grid_size))

        # Define values of grid's objects.
        self.path_fill_value: int = 0
        self.target_fill_value: int = 255
        self.agent_fill_value: int = int(255 * 0.5)
        self.obstacles_fill_value: int = int(255 * 0.25)

        # Define action space.
        # Actions: 0: up, 1: down, 2: left, 3: right.
        self.action_space: gym.spaces.Discrete = gym.spaces.Discrete(4)

        # Define observations space.
        # Observation space: grid_size x grid_size grid.
        self.observation_space = gym.spaces.Box(low=0, high=255, shape=(grid_size, grid_size, 1), dtype=np.uint8)

        # Initialize agent and targe positions.
        self.agent_pos = [0, 0]
        self.target_pos = [grid_size - 1, grid_size - 1]

        # Initialize the grid by resetting it.
        self.reset()

    def _create_grid(self):
        """Create a new grid."""

        grid = np.zeros((self.grid_size, self.grid_size))
        grid.fill(self.obstacles_fill_value)  # Fill grid with obstacles.

        # Add path in the grid.
        for i in range(self.grid_size):
            if (i % self.grid_step == 0):
                grid[i] = np.full((self.grid_size), fill_value=self.path_fill_value)
            for j in range(0, self.grid_size, self.grid_step):
                grid[i][j] = self.path_fill_value

        return grid
    

    def _update_grid(self, new_pos: List[int]):
        """Update grid using new calculated position of agent."""

        self.grid[new_pos[0]][new_pos[1]] = self.agent_fill_value
        self.grid[self.agent_pos[0]][self.agent_pos[1]] = self.path_fill_value
        self.agent_pos = new_pos

    def reset(self, seed: None = None):
        """Reset the state of the environment to an initial state."""

        self.timesteps = 0
        self.agent_pos = [0, 0]

        self.grid = self._create_grid()
        self.grid[self.agent_pos[0]][self.agent_pos[1]] = self.agent_fill_value
        self.grid[self.target_pos[0]][self.target_pos[1]] = self.target_fill_value

        return np.expand_dims(self.grid, axis=-1), {}

    def step(self, action: Any):
        """
        Take an action and return the new state, reward, done, and info.

        :param action: Action to take in the environment.
        """
        raise NotImplementedError
    
    def render(self):
        grid_visual = np.zeros((self.grid_size * 10, self.grid_size * 10, 3), dtype=np.uint8)

        # Draw the grid, obstacles, agent, and target
        for i in range(self.grid_size):
            for j in range(self.grid_size):
                cell_value = self.grid[i][j]
                color = (255, 255, 255)  # Default color for empty cell

                if cell_value == self.obstacles_fill_value:  # Obstacle
                    color = (0, 0, 0)
                elif self.agent_pos == [i, j]:  # Agent
                    color = (0, 255, 0)
                elif self.target_pos == [i, j]:  # Target
                    color = (0, 0, 255)

                cv2.rectangle(grid_visual, (j * 10, i * 10), (j * 10 + 10, i * 10 + 10), color, -1)

        # Display the grid
        cv2.imshow("Grid", grid_visual)
        cv2.waitKey(100)

    def close(self):
        cv2.destroyAllWindows()
