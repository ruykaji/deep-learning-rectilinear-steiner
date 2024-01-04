__all__ = ["GridEnvironment"]

import time
import sys
import gymnasium as gym
import numpy as np

class GridEnvironment(gym.Env):
    """
    Custom Environment that follows gym interface.
    This is a simple grid environment where the agent has to reach a target cell in a grid with obstacles.
    """

    def __init__(self, grid_size=84, grid_step=2):
        super(GridEnvironment, self).__init__()

        self.render_mode = "human"
        self.is_first_render = True
        self.total_timesteps = 0
        self.total_timesteps_thr = grid_size * 10
        self.target_fill_value = 255
        self.agent_fill_value = 125
        self.obstacles_fill_value = 62
        self.path_fill_value = 0

        # Define action and observation space
        # Actions: 0: up, 1: down, 2: left, 3: right
        self.action_space = gym.spaces.Discrete(4)

        # Observation space: grid_size x grid_size grid
        self.observation_space = gym.spaces.Box(low=0, high=255, shape=(grid_size, grid_size, 1), dtype=np.uint8)

        self.grid_size = grid_size
        self.grid_step = grid_step
        self.agent_pos = [0, 0]  # Starting position
        self.target_pos = [grid_size - 1, grid_size - 1]  # Target position

        # Initialize the grid
        self.grid = self._create_grid(grid_size, grid_step)
        self.grid[self.agent_pos[0]][self.agent_pos[1]] = self.agent_fill_value
        self.grid[self.target_pos[0]][self.target_pos[1]] = self.target_fill_value

    def _create_grid(self, size, step):
        """Create a new grid."""

        grid = np.zeros((size, size))
        grid.fill(self.obstacles_fill_value)  # Fill grid with obstacles

        # Add path in the grid
        for i in range(size):
            if(i % step == 0):
                    grid[i] = np.full((self.grid_size), fill_value=self.path_fill_value)
            for j in range(0, size, step):
                grid[i][j] = self.path_fill_value

        return grid

    def reset(self, seed = None):
        """Reset the state of the environment to an initial state."""

        self.is_first_render = True
        self.total_timesteps = 0
        self.agent_pos = [0, 0]

        self.grid[self.agent_pos[0]][self.agent_pos[1]] = self.agent_fill_value
        self.grid[self.target_pos[0]][self.target_pos[1]] = self.target_fill_value

        self.grid = self._create_grid(self.grid_size, self.grid_step)

        return np.expand_dims(self.grid, axis=-1), {}

    def step(self, action):
        """Take an action and return the new state, reward, done, and info."""

        # Calculate new position
        new_pos = self.agent_pos.copy()

        if action == 0 and self.agent_pos[0] > 0:
            new_pos[0] -= 1
        elif action == 1 and self.agent_pos[0] < self.grid_size - 1:
            new_pos[0] += 1
        elif action == 2 and self.agent_pos[1] > 0:
            new_pos[1] -= 1
        elif action == 3 and self.agent_pos[1] < self.grid_size - 1:
            new_pos[1] += 1

        # Check if new position is an obstacle
        if self.grid[new_pos[0], new_pos[1]] == self.obstacles_fill_value:
            reward = -1  # Penalty for hitting an obstacle
        else:
            self.agent_pos = new_pos
            reward = 0

        # Check if target is reached
        terminated = self.agent_pos == self.target_pos

        if terminated:
            reward = 1  # Reward for reaching the target

        if (reward != -1):
            # Update grid with new agent position
            self.grid = self._create_grid(self.grid_size, self.grid_step)
            self.grid[self.agent_pos[0], self.agent_pos[1]] = self.agent_fill_value

        self.total_timesteps += 1

        if(self.total_timesteps > self.total_timesteps_thr):
            reward = -1
            terminated = True

        return np.expand_dims(self.grid, axis=-1), reward, terminated, False, {}

    def render(self):
        # Move the cursor up by grid_size lines
        if(not self.is_first_render):
            print("\033[F" * self.grid_size, end='')
        else:
            print()
            self.is_first_render = False

        # Create an empty grid for rendering as strings
        grid_render = np.full((self.grid_size, self.grid_size), ' ', dtype='<U1')

        # Mark obstacles, agent, and target in the grid
        for i in range(self.grid_size):
            for j in range(self.grid_size):
                if self.grid[i, j] == self.path_fill_value:
                    grid_render[i, j] = '='  # '=' represents an obstacle
                elif (i, j) == tuple(self.agent_pos):
                    grid_render[i, j] = 'A'  # 'A' represents the agent
                elif (i, j) == tuple(self.target_pos):
                    grid_render[i, j] = 'T'  # 'T' represents the target

        # Print each row of the grid
        for i in range(self.grid_size):
            print(' '.join(grid_render[i]))