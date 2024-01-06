import argparse
from stable_baselines3 import PPO, DQN
from stable_baselines3.common.env_util import make_vec_env
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
        "ent_coef": 0.01,
        "vf_coef": 0.5
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
        "eval_freq": 1000,
        "render": True,
    }
}

algo = {
    "ppo": lambda policy, env: PPO(policy, env, verbose=1, **hyperparams["ppo"]),
    "dqn": lambda policy, env: DQN(policy, env, verbose=1, **hyperparams["dqn"]),
}


def main(args) -> None:
    for key in algo.keys():
        log_folder = "./logs/" + key

        for grid_size in range(args.bot_grid_size, args.top_grid_size + 8, 8):
            # Create environments
            env = make_vec_env(lambda: envs.Simple2DGridEnv(grid_size=grid_size, grid_step=args.grid_step), n_envs=args.num_envs)
            eval_env = VecTransposeImage(make_vec_env(lambda: envs.Simple2DGridEnv(grid_size=grid_size, grid_step=args.grid_step), n_envs=1))

            # Create the model
            model = algo[key]("MlpPolicy", env)

            # Train and eval the model
            model.learn(total_timesteps=args.total_timesteps, callback=utils.EvalCallback(eval_env, log_folder + "/grid_size_" + str(grid_size), **hyperparams["eval"]))

            utils.plot_eval(log_folder, log_folder)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Train a RL agent on a custom grid environment")
    parser.add_argument("--num_envs", type=int, default=16, help="Number of environments.")
    parser.add_argument("--total_timesteps", type=int, default=2e6, help="Total timesteps for training.")
    parser.add_argument("--grid_step", type=int, default=2, help="Size of the grid's col and row step environment.")
    parser.add_argument("--bot_grid_size", type=int, default=16, help="Bottom plank of size of the grid environment.")
    parser.add_argument("--top_grid_size", type=int, default=32, help="Top plank of size of the grid environment.")

    main(parser.parse_args())
