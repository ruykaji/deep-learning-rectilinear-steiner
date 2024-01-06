__all__ = ["EvalCallback"]

import os
import numpy as np
from stable_baselines3.common.callbacks import BaseCallback
from stable_baselines3.common.logger import configure

class EvalCallback(BaseCallback):
    """
    Evolution callback for proper normalization of rewards and episode lengths.

    :param eval_env: Env to evaluate.
    :param folder: Path to save folder.
    :param eval_freq: Eval frequency
    :param render: Render or not evolution process. 
    """
    def __init__(self, eval_env, folder, eval_freq,  render, verbose=1):
        super(EvalCallback, self).__init__(verbose)

        self.eval_env = eval_env
        self.eval_freq = eval_freq
        self.render = render
        self.eval_logger = configure(folder=folder, format_strings=["stdout", "csv"])
        self.eval_rewards = []
        self.eval_lengths = []

        if (not os.path.exists(folder)):
            os.makedirs(folder, exist_ok=True)

    def _on_step(self) -> bool:
        if self.n_calls % self.eval_freq == 0:
            n_envs = self.eval_env.num_envs
            dones = np.zeros((n_envs, ), dtype=bool)
            current_rewards = np.zeros(n_envs)
            current_lengths = np.zeros(n_envs, dtype="int")
            observations = self.eval_env.reset()
            states = None
            episode_starts = np.ones((n_envs,), dtype=bool)

            while not dones.any():
                actions, states = self.model.predict(
                    observations,
                    state=states,
                    episode_start=episode_starts,
                    deterministic=True,
                )

                new_observations, rewards, dones, _ = self.eval_env.step(actions)

                current_rewards += rewards
                current_lengths += 1

                observations = new_observations

                if self.render:
                    self.eval_env.render()

            info = self.eval_env.env_method("get_info")

            episode_mean_reward = []
            episode_mean_length = []

            for i in range(n_envs):
                episode_mean_reward.append((current_rewards[i] - info[i]["min_episode_reward"]) / (info[i]["max_episode_reward"] - info[i]["min_episode_reward"]))
                episode_mean_length.append((current_lengths[i] - info[i]["min_episode_length"]) / (info[i]["max_episode_length"] - info[i]["min_episode_length"]))

            self.eval_rewards.append(sum(episode_mean_reward) / len(episode_mean_reward))
            self.eval_lengths.append(sum(episode_mean_length) / len(episode_mean_length))
            self.log_info()

        return True

    def log_info(self):
        self.eval_logger.record("reward", self.eval_rewards[-1])
        self.eval_logger.record("episode_length", self.eval_lengths[-1])
        self.eval_logger.record("total_timesteps", self.num_timesteps)
        self.eval_logger.dump(self.n_calls)
