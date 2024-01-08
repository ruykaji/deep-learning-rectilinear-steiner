__all__ = ["__DEFAULT_HYPERPARAMS__", "update_hyperparams"]

__DEFAULT_HYPERPARAMS__ = {
    "ppo": {
        "learning_rate": 0.00025,
        "n_steps": 128,
        "n_epochs": 4,
        "batch_size": 256,
        "clip_range": 0.1,
        "ent_coef": 0.01,
        "vf_coef": 0.5,
        "max_grad_norm": 0.5
    },
    "dqn": {
        "learning_rate": 0.0001,
        "buffer_size": 100000,
        "learning_starts": 100000,
        "target_update_interval": 1000,
        "train_freq": 4,
        "gradient_steps": 1,
        "exploration_fraction": 0.1,
        "exploration_final_eps": 0.01,
        "max_grad_norm": 10.0
    },
}


def update_hyperparams(algo, hyperparams, trial):
    """Update global hyperparametes..."""

    suggestions = dict()

    if algo == "dqn":
        suggestions["learning_rate"] = trial.suggest_float("learning_rate", 1e-5, 1e-3, log=True)
        suggestions["buffer_size"] = trial.suggest_int("buffer_size", 100000, 1000000)
        suggestions["learning_starts"] = trial.suggest_int("learning_starts", 10000, 50000)
        suggestions["target_update_interval"] = trial.suggest_int("target_update_interval", 10000, 50000)
        suggestions["max_grad_norm"] = trial.suggest_float("max_grad_norm", 0.5, 10.0, log=True)

        hyperparams["dqn"].update(suggestions)
    elif algo == "ppo":
        suggestions["learning_rate"] = trial.suggest_float("learning_rate", 1e-5, 1e-3, log=True)
        suggestions["n_steps"] = trial.suggest_int("n_steps", 128, 256)
        suggestions["n_epochs"] = trial.suggest_int("n_epochs", 4, 10)
        suggestions["batch_size"] = trial.suggest_int("batch_size", 32, 256)
        suggestions["clip_range"] = trial.suggest_float("clip_range", 0.01, 0.2, log=True)
        suggestions["ent_coef"] = trial.suggest_float("ent_coef", 0.01, 0.1, log=True)
        suggestions["vf_coef"] = trial.suggest_float("vf_coef", 0.1, 0.5, log=True)
        suggestions["max_grad_norm"] = trial.suggest_float("max_grad_norm", 0.1, 1.0, log=True)

        hyperparams["ppo"].update(suggestions)
