import argparse
from stable_baselines3 import PPO, DQN
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.logger import configure
from stable_baselines3.common.callbacks import EvalCallback
from stable_baselines3.common.vec_env import VecTransposeImage

import envs
import utils


hyperparams = {
    "ppo": {
        "learning_rate": 0.00025,
        "n_steps": 128,
        "n_epochs": 4,
        "batch_size": 256,
        "clip_range": 0.1,
        "ent_coef": 0.01
    },
    "dqn": {
        "learning_rate": 0.0001,
        "buffer_size": 100000,
        "learning_starts": 100000,
        "target_update_interval": 1000,
        "train_freq": 4,
        "gradient_steps": 1,
        "exploration_fraction": 0.1,
        "exploration_final_eps": 0.01
    },
    "eval": {
        "n_eval_episodes": 1,
        "eval_freq": 10000,
        "render": True,
    }
}

algo = {
    "ppo": lambda policy, env: PPO(policy, env, verbose=1, **hyperparams["ppo"]),
    "dqn": lambda policy, env: DQN(policy, env, verbose=1, **hyperparams["dqn"]),
}


def main(args):
    for key in algo.keys():
        log_folder = "./logs/" + key

        for grid_size in range(args.bot_grid_size, args.top_grid_size, 4):
            logger = configure(log_folder + "/grid_size_" + str(grid_size), ["stdout", "csv"])

            # Create environments
            env = make_vec_env(lambda: envs.Simple2DGridEnv(grid_size=grid_size), n_envs=args.num_envs)
            eval_env = VecTransposeImage(make_vec_env(lambda: envs.Simple2DGridEnv(grid_size=grid_size), n_envs=1))

            # Create the model
            model = algo[key]("MlpPolicy", env)

            # Train and eval the model
            model.set_logger(logger)
            model.learn(total_timesteps=args.total_timesteps, callback=EvalCallback(eval_env, **hyperparams["eval"]))

            utils.plot_train(log_folder, log_folder)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Train a RL agent on a custom grid environment")
    parser.add_argument("--num_envs", type=int, default=16, help="Number of environments.")
    parser.add_argument("--total_timesteps", type=int, default=2e6, help="Total timesteps for training.")
    parser.add_argument("--bot_grid_size", type=int, default=16, help="Bottom plank of size of the grid environment.")
    parser.add_argument("--top_grid_size", type=int, default=32, help="Top plank of size of the grid environment.")

    main(parser.parse_args())
