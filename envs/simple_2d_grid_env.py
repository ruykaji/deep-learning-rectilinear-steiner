__all__ = ["Simple2DGridEnv"]

from typing import Tuple

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

    def callback_step(self, action) -> Tuple[float, bool, bool]:
        """
        Take an action and return the new state, reward, done, and info.

        :param action: Action to take in the environment.
        """

        is_stuck = False
        truncated = False
        terminated = False
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
            reward = -0.015  # Penalty for getting over a bounds.
        elif self.grid[new_pos[0], new_pos[1]] == self.obstacles_fill_value:
            reward = -0.010  # Penalty for hitting an obstacle
        elif self.grid[new_pos[0], new_pos[1]] == self.track_fill_value:
            reward = -0.05  # Penalty for hitting an obstacle
        else:
            reward = -0.001  # Penalty for wondering around
            update_state = True  # Update valid state

        # Update grid with new agent position
        if (update_state):
            self._update_grid(new_pos)
            is_stuck = self._is_stuck()

        # Check if target is reached
        if self.agent_pos == self.target_pos:
            reward = 1.0  # Reward for reaching the target
            terminated = True

        if (not terminated and (is_stuck or self.timesteps >= self.max_episode_length)):
            reward = -1.0
            terminated = True
            truncated = True

        return reward, terminated, truncated

    def callback_reset(self, seed=None) -> None:
        self.max_episode_reward = self.min_episode_length * -0.001 + 1.0
        self.min_episode_reward = self.max_episode_length * -0.015 - 1.0
