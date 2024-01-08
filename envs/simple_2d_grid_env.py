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

        self.win_reward = 1.0
        self.loss_penalty = -1.0
        self.inefficiency_penalty = -0.001
        self.collision_penalty = -0.05

    def callback_step(self, action) -> Tuple[float, bool, bool]:
        """
        Take an action and return the new state, reward, done, and info.

        :param action: Action to take in the environment.
        """

        is_stuck = False
        truncated = False
        terminated = False
        update_sate = False

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

        reward = self.inefficiency_penalty

        # Check if new position is an obstacle
        if new_pos[0] < 0 or new_pos[0] > self.grid_size - 1 or new_pos[1] < 0 or new_pos[1] > self.grid_size - 1:
            reward = self.collision_penalty
            is_stuck = True
        elif self.grid[new_pos[0], new_pos[1]] == self.obstacles_fill_value or self.grid[new_pos[0], new_pos[1]] == self.track_fill_value:
            reward = self.collision_penalty
            is_stuck = True
        else:
            update_sate = True

        # Check if target is reached
        if new_pos == self.target_pos:
            reward = self.win_reward
            terminated = True
        elif (is_stuck):
            self.stuck_time += 1

            if (self.stuck_time >= self.max_stuck_time):
                reward = self.loss_penalty
                truncated = True

        # Update grid with new agent position
        if (update_sate):
            self.stuck_time = 0
            self._update_grid(new_pos)

        return reward, terminated, truncated

    def callback_reset(self, seed=None) -> None:
        pass
