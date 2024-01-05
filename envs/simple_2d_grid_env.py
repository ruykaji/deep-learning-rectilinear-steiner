__all__ = ["Simple2DGridEnv"]

import numpy as np

from .base_grid_env import BaseGridEnv


class Simple2DGridEnv(BaseGridEnv):
    """
    Simple 2D grid environment.
    This environment does not create additional obstacles beside that already created by rows and columns.

    :param grid_size: Size of the grid.
    :param grid_step: Step size between each col and row in grid.
    """

    def __init__(self, grid_size=16, grid_step=2):
        super().__init__(grid_size, grid_step)

    def step(self, action):
        """
        Take an action and return the new state, reward, done, and info.

        :param action: Action to take in the environment.
        """

        update_state = False

        # Calculate new position
        new_pos = self.agent_pos.copy()

        if action == 0:
            new_pos[0] -= 1
        elif action == 1:
            new_pos[0] += 1
        elif action == 2:
            new_pos[1] -= 1
        elif action == 3:
            new_pos[1] += 1

        # Check if new position is an obstacle
        if (new_pos[0] < 0 or new_pos[0] > self.grid_size - 1 or new_pos[1] < 0 or new_pos[1] > self.grid_size - 1):
            reward = -1  # Penalty for getting over a bounds.
        elif self.grid[new_pos[0], new_pos[1]] == self.obstacles_fill_value:
            reward = -0.75  # Penalty for hitting an obstacle
        else:
            reward = -0.04  # Penalty for wondering around
            update_state = True # Update valid state

        # Check if target is reached
        terminated = new_pos == self.target_pos
        truncated = False

        if terminated:
            reward = 1  # Reward for reaching the target

        self.timesteps += 1

        if (self.timesteps > self.timesteps_thr and not terminated):
            reward = -1
            truncated = True

        # Update grid with new agent position
        if (update_state):
            self._update_grid(new_pos)

        return np.expand_dims(self.grid, axis=-1), reward, terminated, truncated, {}
