__all__ = ["BaseGridEnv"]

from typing import Tuple, Dict, Any
import cv2
import gymnasium as gym
import numpy as np
import random
from collections import deque


class BaseGridEnv(gym.Env):
    """
    Base grid environment.

    :param grid_size: Size of the grid.
    :param grid_step: Step size between each col and row in grid.
    """

    def __init__(self, grid_size: int = 16, grid_step: int = 2) -> None:
        super().__init__()

        self.render_mode = "human"

        self.grid_size: int = grid_size
        self.grid_step: int = grid_step

        # Define time-step counter and threshold time-step value.
        self.timesteps: int = 0

        # Define values of grid's objects.
        self.path_fill_value: int = 0
        self.target_fill_value: int = 255
        self.track_fill_value: int = int(255 * 0.5)
        self.agent_fill_value: int = int(255 * 0.75)
        self.obstacles_fill_value: int = int(255 * 0.25)

        # Define parameters
        self.agent_pos = [0, 0]
        self.target_pos = [0, 0]

        # Define action space.
        # Actions: 0: up, 1: down, 2: left, 3: right.
        self.action_space: gym.spaces.Discrete = gym.spaces.Discrete(4)

        # Define observations space.
        # Observation space: grid_size x grid_size grid.
        self.observation_space = gym.spaces.Box(low=0, high=255, shape=(grid_size, grid_size, 1), dtype=np.uint8)

        # Initialize the grid by resetting it.
        self.max_episode_length: float = int(grid_size * grid_size * 0.5)
        self.min_episode_length: float = 0.0
        self.max_episode_reward: float = 0.0
        self.min_episode_reward: float = 0.0

        self.reset()

    def _set_random_positions(self) -> None:
        """
        Set the agent and target in random positions on the grid.
        """
        valid_positions = [(i, j) for i in range(self.grid_size) for j in range(self.grid_size)
                           if self.grid[i][j] == self.path_fill_value]

        # Randomly choose two different positions from the list of valid positions
        self.agent_pos, self.target_pos = random.sample(valid_positions, 2)

        self.agent_pos = list(self.agent_pos)
        self.target_pos = list(self.target_pos)
        self.new_agent_pos = self.agent_pos.copy()

        # Set the agent and target on the grid
        self.grid[self.agent_pos[0]][self.agent_pos[1]] = self.agent_fill_value
        self.grid[self.target_pos[0]][self.target_pos[1]] = self.target_fill_value

    def _create_grid(self) -> np.ndarray:
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

    def _update_grid(self, new_pos) -> None:
        """Update grid using new calculated position of agent."""

        if (new_pos != self.agent_pos):
            self.grid[new_pos[0]][new_pos[1]] = self.agent_fill_value
            self.grid[self.agent_pos[0]][self.agent_pos[1]] = self.track_fill_value
            self.agent_pos = new_pos

    def _find_shortest_path(self) -> int:
        """
        The Lee algorithm.
        """

        def is_valid(x, y, matrix):
            return 0 <= x < self.grid_size and 0 <= y < self.grid_size and matrix[x][y] == 0 and (self.grid[x][y] == self.path_fill_value or self.grid[x][y] == self.target_fill_value)

        src = self.agent_pos.copy()
        dest = self.target_pos.copy()
        matrix = np.zeros((self.grid_size, self.grid_size))

        # Directions for moving up, down, left, right
        directions = [(0, 1), (1, 0), (0, -1), (-1, 0)]

        # Queue for BFS
        q = deque()
        q.append(src)

        while q:
            x, y = q.popleft()

            # Check if we reached destination
            if [x, y] == dest:
                return matrix[x][y] - 1

            for dx, dy in directions:
                nx, ny = x + dx, y + dy

                # Check if the new position is valid
                if is_valid(nx, ny, matrix):
                    matrix[nx][ny] = matrix[x][y] + 1
                    q.append((nx, ny))

        # Destination cannot be reached
        raise ValueError("Can't find path between agent and target!")

    def _is_stuck(self) -> bool:
        pos = self.agent_pos.copy()
        left = True
        right = True
        bottom = True
        top = True

        if (pos[0] == 0 or self.grid[pos[0] - 1, pos[1]] == self.obstacles_fill_value or self.grid[pos[0] - 1, pos[1]] == self.track_fill_value):
            left = False

        if (pos[0] == self.grid_size - 1 or self.grid[pos[0] + 1, pos[1]] == self.obstacles_fill_value or self.grid[pos[0] + 1, pos[1]] == self.track_fill_value):
            right = False

        if (pos[1] == 0 or self.grid[pos[0], pos[1] - 1] == self.obstacles_fill_value or self.grid[pos[0], pos[1] - 1] == self.track_fill_value):
            top = False

        if (pos[1] == self.grid_size - 1 or self.grid[pos[0], pos[1] + 1] == self.obstacles_fill_value or self.grid[pos[0], pos[1] + 1] == self.track_fill_value):
            bottom = False

        if (left or right or bottom or top):
            return False

        return True

    def get_info(self) -> Dict[str, float]:
        """
        Return info abut length and reward in the current grid.
        """
        return {
            "min_episode_length": self.min_episode_length,
            "max_episode_length": self.max_episode_length,
            "min_episode_reward": self.min_episode_reward,
            "max_episode_reward": self.max_episode_reward,
        }

    def step(self, action: Any) -> Tuple[np.ndarray, float, bool, bool, Dict[Any, Any]]:
        """
        Take an action and return the new state, reward, done, and info.

        :param action: Action to take in the environment.
        """

        self.timesteps += 1
        reward, terminated, truncated = self.callback_step(action)

        return np.expand_dims(self.grid, axis=-1), reward, terminated, truncated, {}

    def reset(self, seed=None) -> Tuple[np.ndarray, Dict[Any, Any]]:
        """Reset the state of the environment to an initial state."""

        super().reset(seed=seed)

        self.timesteps = 0
        self.grid = self._create_grid()
        self._set_random_positions()
        self.min_episode_length: float = self._find_shortest_path()

        # Call derived reset callback.
        self.callback_reset(seed)

        return np.expand_dims(self.grid, axis=-1), {}

    def callback_step(self, action: Any) -> Tuple[float, bool, bool]:
        """
        Derived class should override this method.
        Take an action and return the new state, reward, done, and info.

        :param action: Action to take in the environment.
        """
        raise NotImplementedError

    def callback_reset(self, seed=None) -> None:
        """
        Derived class should override this method.
        Reset the state of the environment to an initial state.
        """
        raise NotImplementedError

    def render(self) -> None:
        grid_visual = np.zeros((self.grid_size * 10, self.grid_size * 10, 3), dtype=np.uint8)

        # Draw the grid, obstacles, agent, and target
        for i in range(self.grid_size):
            for j in range(self.grid_size):
                cell_value = self.grid[i][j]
                color = (255, 255, 255)  # Default color for empty cell

                if cell_value == self.obstacles_fill_value:  # Obstacle
                    color = (0, 0, 0)
                elif cell_value == self.track_fill_value:
                    color = (255, 0, 0)
                elif self.agent_pos == [i, j]:  # Agent
                    color = (0, 255, 0)
                elif self.target_pos == [i, j]:  # Target
                    color = (0, 0, 255)

                cv2.rectangle(grid_visual, (j * 10, i * 10), (j * 10 + 10, i * 10 + 10), color, -1)

        # Display the grid
        cv2.imshow("Grid", grid_visual)
        cv2.waitKey(5)

    def close(self) -> None:
        cv2.destroyAllWindows()
