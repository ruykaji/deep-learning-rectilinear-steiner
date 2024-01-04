import argparse
from stable_baselines3 import PPO
from stable_baselines3.common.env_util import make_vec_env
from grid_environment import GridEnvironment
from stable_baselines3.common.callbacks import EvalCallback
from stable_baselines3.common.vec_env import VecTransposeImage

def main(args):
    # Create the environment
    env = make_vec_env(lambda: GridEnvironment(grid_size=args.grid_size, grid_step=args.step_size), n_envs=args.num_envs)
    eval_env =VecTransposeImage(make_vec_env(lambda: GridEnvironment(grid_size=args.grid_size, grid_step=args.step_size), n_envs=1))

    # Choose the model
    if args.algo == "ppo":
        model = PPO("MlpPolicy", env, verbose=1, learning_rate=args.lr, n_steps=args.n_steps)

    # Train the model
    model.learn(total_timesteps=args.total_timesteps, callback=EvalCallback(eval_env, best_model_save_path="./logs/", log_path="./logs/", eval_freq=10000, render=True))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Train a RL agent on a custom grid environment")
    parser.add_argument("--num_envs", type=int, default=16, help="Number of environments.")
    parser.add_argument("--grid_size", type=int, default=32, help="Size of the grid environment")
    parser.add_argument("--step_size", type=int, default=2, help="Size of the step in grid environment")
    parser.add_argument("--total_timesteps", type=int, default=1e6, help="Total timesteps for training")
    parser.add_argument("--algo", type=str, default="ppo", choices=["ppo"], help="RL algorithm to use")
    parser.add_argument("--lr", type=float, default=0.001, help="Learning rate")
    parser.add_argument("--n_steps", type=int, default=2048, help="Number of steps to run for each environment per update")

    main(parser.parse_args())