import argparse
import os
import optuna
import torch
from stable_baselines3 import PPO, DQN
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.logger import configure
from stable_baselines3.common.vec_env import VecTransposeImage, VecFrameStack
from stable_baselines3.common.callbacks import StopTrainingOnNoModelImprovement, EvalCallback
from stable_baselines3.common.evaluation import evaluate_policy

from envs import Simple2DGridEnv
from utils import plot_train, update_hyperparams, __DEFAULT_HYPERPARAMS__

__LOCAL_HYPERPARAMS__ = __DEFAULT_HYPERPARAMS__.copy()

__ALGO__ = {
    "dqn": lambda policy, env, **kwargs: DQN(policy, env, verbose=1, **kwargs),
    "ppo": lambda policy, env, **kwargs: PPO(policy, env, verbose=1, **kwargs),
}


def train(args, log_folder: str,  grid_size: int, eval_on_end: bool = False) -> float:
    # Create logger
    logger = configure(os.path.join(log_folder, "grid_size_" + str(grid_size)), ["stdout", "csv"])

    # Create environments
    env = VecFrameStack(make_vec_env(lambda: Simple2DGridEnv(grid_size=grid_size, grid_step=args.grid_step), n_envs=args.num_envs), n_stack=4)
    eval_env = VecTransposeImage(VecFrameStack(make_vec_env(lambda: Simple2DGridEnv(grid_size=grid_size, grid_step=args.grid_step), n_envs=1), n_stack=4))

    # Create the model
    model = __ALGO__.get(args.algo)("MlpPolicy", env, **__LOCAL_HYPERPARAMS__[args.algo])

    model.set_logger(logger)
    model.learn(total_timesteps=args.total_timesteps, log_interval=1, callback=EvalCallback(eval_env, eval_freq=10000, n_eval_episodes=1, render=False,
                callback_after_eval=StopTrainingOnNoModelImprovement(max_no_improvement_evals=5, min_evals=10, verbose=1)))

    if (eval_on_end):
        mean_reward, _ = evaluate_policy(model, eval_env, n_eval_episodes=10)
        return mean_reward
    return 0.0

def main(args) -> None:
    if (args.tune):
        log_folder = os.path.join("./logs/hyperparams", args.algo)

        for grid_size in range(args.bot_grid_size, args.top_grid_size + 8, 8):
            def objective(trial):
                update_hyperparams(args.algo, __LOCAL_HYPERPARAMS__, trial)
                mean_reward = train(args, log_folder, grid_size, True)
                return mean_reward

            study = optuna.create_study(direction='maximize')
            study.optimize(objective, n_trials=25)

            with open(os.path.join(log_folder, "grid_size_" + str(grid_size), "hyperparams.txt"), "w") as f:
                for key in study.best_trial.params:
                    f.write(key + ": " + str(study.best_trial.params[key]) + "\n")
                f.write(str(study.best_value))

            plot_train(log_folder, log_folder)
    else:
        log_folder = os.path.join("./logs/train", args.algo)

        for grid_size in range(args.bot_grid_size, args.top_grid_size + 8, 8):
            train(args, log_folder, grid_size)
            plot_train(log_folder, log_folder)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Train a RL agent on a custom grid environment")

    parser.add_argument("--tune", type=bool, default=True, help="Algorithm to use.")
    parser.add_argument("--algo", choices=["dqn", "ppo"], default="dqn", help="Algorithm to use.")
    parser.add_argument("--num_envs", type=int, default=16, help="Number of environments.")
    parser.add_argument("--total_timesteps", type=int, default=2e6, help="Total timesteps for training.")
    parser.add_argument("--grid_step", type=int, default=2, help="Size of the grid's col and row step environment.")
    parser.add_argument("--bot_grid_size", type=int, default=16, help="Bottom plank of size of the grid environment.")
    parser.add_argument("--top_grid_size", type=int, default=32, help="Top plank of size of the grid environment.")

    torch.manual_seed(42)

    main(parser.parse_args())
