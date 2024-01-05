__all__ = ["plot_evaluations"]

import os
import numpy as np
import matplotlib.pyplot as plt


def plot_evaluations(path_to_evaluations: str = None, save_path: str = None):
    """
    Plots evaluations data.

    :param path_to_evaluations: Path to folder with experiment/evaluations.npz folders.
    :param save_path: Path to save plot.
    """

    if (path_to_evaluations != None and os.path.exists(path_to_evaluations)):
        dirs = os.listdir(path_to_evaluations)

        if (len(dirs) != 0):
            plt.xlabel('Timesteps')
            plt.ylabel('Mean Reward')
            plt.title('Evaluation Results Over Time')

            for dir in dirs:
                data = np.load(os.path.join(path_to_evaluations, dir, "evaluations.npz"), allow_pickle=True)
                evaluation_results = data['results']
                evaluation_timesteps = data['timesteps']

                plt.legend(dir)
                plt.plot(evaluation_timesteps, evaluation_results)

            if (save_path != None):
                plt.savefig(os.path.join(save_path, "evaluations.png"), dpi = 400)
            else:
                plt.show()

            plt.close()
