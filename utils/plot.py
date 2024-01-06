__all__ = ["plot_eval"]

import os
import pandas as pd
import matplotlib.pyplot as plt


def plot_eval(path_to_eval_csv: str = None, save_path: str = None):
    """
    Plots evaluations data.

    :param path_to_eval_csv: Path to folder with experiment/progress.csv folders.
    :param save_path: Path to save plot.
    """

    if (path_to_eval_csv != None and os.path.exists(path_to_eval_csv)):
        dirs = os.listdir(path_to_eval_csv)
        legend = []

        if (len(dirs) != 0):
            # Rewards from time-step
            plt.xlabel('Timesteps')
            plt.ylabel('Mean Reward')
            plt.title('Evaluation Results Over Time')

            for dir in dirs:
                if (os.path.isdir(os.path.join(path_to_eval_csv, dir))):
                    legend.append(dir)
                    data = data = pd.read_csv(os.path.join(path_to_eval_csv, dir, "progress.csv"))
                    plt.plot(data['total_timesteps'], data["reward"])

            plt.legend(legend)

            if (save_path != None):
                plt.savefig(os.path.join(save_path, "timestep_reward.png"), dpi=400)
            else:
                plt.show()

            plt.close()

            # Episode length from time-step
            plt.xlabel('Timesteps')
            plt.ylabel('Mean Episode Length')
            plt.title('Evaluation Results Over Time')

            for dir in dirs:
                if (os.path.isdir(os.path.join(path_to_eval_csv, dir))):
                    legend.append(dir)
                    data = data = pd.read_csv(os.path.join(path_to_eval_csv, dir, "progress.csv"))
                    plt.plot(data['total_timesteps'], data["episode_length"])

            plt.legend(legend)

            if (save_path != None):
                plt.savefig(os.path.join(save_path, "timestep_episode_length.png"), dpi=400)
            else:
                plt.show()

            plt.close()
