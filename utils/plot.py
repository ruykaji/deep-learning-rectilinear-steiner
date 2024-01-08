__all__ = ["plot_train"]

import os
import pandas as pd
import matplotlib.pyplot as plt


def plot_train(path_to_train_csv: str = None, save_path: str = None):
    """
    Plots evaluations data.

    :param path_to_train_csv: Path to folder with experiment/progress.csv folders.
    :param save_path: Path to save plot.
    """

    if (path_to_train_csv != None and os.path.exists(path_to_train_csv)):
        dirs = os.listdir(path_to_train_csv)
        legend = []

        if (len(dirs) != 0):
            plt.xlabel('Timesteps')
            plt.ylabel('Mean Reward')
            plt.title('Evaluation Results Over Time')

            for dir in dirs:
                if(os.path.isdir(os.path.join(path_to_train_csv, dir))):
                    legend.append(dir)
                    data = data = pd.read_csv(os.path.join(path_to_train_csv, dir, "progress.csv"))
                    plt.plot(data['time/total_timesteps'], data["rollout/ep_rew_mean"])

            plt.legend(legend)
            
            if (save_path != None):
                plt.savefig(os.path.join(save_path, "progress.png"), dpi = 400)
            else:
                plt.show()

            plt.close()
