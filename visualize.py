import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


def _font_profile(multiplier: float):
    base = {
        "title": 12,
        "label": 11,
        "tick": 10,
        "legend": 10,
        "suptitle": 16,
    }
    return {k: max(1, int(v * multiplier)) for k, v in base.items()}

COLORS = {
    "ShiftToMiddleArray": "#d62728",
    "ExpandingRingBuffer": "#2ca02c",
    "std::vector": "#1f77b4",
    "std::deque": "#1f77b4",
    "std::queue": "#1f77b4",
}


def _relative_with_error(means, stds):
    best_idx = np.argmin(means)
    best = means[best_idx]
    rel = (means / best) * 100.0

    # conservative propagation for ratio X/B
    rel_std = np.zeros_like(rel)
    best_std = stds[best_idx]
    for i in range(len(means)):
        if means[i] == 0 or best == 0:
            rel_std[i] = 0
        else:
            rel_std[i] = rel[i] * np.sqrt((stds[i] / means[i]) ** 2 + (best_std / best) ** 2)
    return rel, rel_std


def plot_two_col_schema(file_path, out_path, font_mult=1.0):
    df = pd.read_csv(file_path)
    sizes = sorted(df["Size"].unique())
    containers = list(df["Type"].unique())
    fp = _font_profile(font_mult)

    bar_w = 0.23
    x = np.arange(len(sizes))
    plt.figure(figsize=(11, 6))

    for j, container in enumerate(containers):
        rel_vals, rel_err = [], []
        for size in sizes:
            subset = df[df["Size"] == size]
            means = subset["TimeMeanMs"].values
            stds = subset["TimeStdMs"].values
            rel, rel_std = _relative_with_error(means, stds)
            idx = subset["Type"].tolist().index(container)
            rel_vals.append(rel[idx])
            rel_err.append(rel_std[idx])

        plt.bar(
            x + (j - (len(containers) - 1) / 2) * bar_w,
            rel_vals,
            yerr=rel_err,
            capsize=3,
            width=bar_w,
            label=container,
            color=COLORS.get(container, "gray"),
            alpha=0.9,
        )

    plt.xticks(x, sizes, rotation=45, fontsize=fp["tick"])
    plt.xlabel("Benchmark Result Size", fontsize=fp["label"])
    plt.ylabel("Relative Time (% of best at each size)", fontsize=fp["label"])
    plt.title(os.path.basename(file_path).replace("_", " ").replace(".csv", ""), fontsize=fp["title"])
    plt.legend(fontsize=fp["legend"])
    plt.grid(axis="y", linestyle="--", alpha=0.3)
    plt.tight_layout()
    plt.savefig(out_path, dpi=220)
    plt.close()
    print(f"Saved visualization: {out_path}")


def plot_queue_schema(file_path, out_path, font_mult=1.0):
    df = pd.read_csv(file_path)
    workloads = [
        ("PushHeavyMeanMs", "PushHeavyStdMs", "Push-heavy (80/20)"),
        ("MixedMeanMs", "MixedStdMs", "Mixed (50/50)"),
        ("PopHeavyMeanMs", "PopHeavyStdMs", "Pop-heavy (20/80)"),
    ]
    sizes = sorted(df["Size"].unique())
    containers = list(df["Type"].unique())
    fp = _font_profile(font_mult)

    fig, axes = plt.subplots(3, 1, figsize=(16, 15), sharex=True)
    bar_w = 0.22
    x = np.arange(len(sizes))

    for ax, (mean_col, std_col, title) in zip(axes, workloads):
        for j, container in enumerate(containers):
            rel_vals, rel_err = [], []
            for size in sizes:
                subset = df[df["Size"] == size]
                means = subset[mean_col].values
                stds = subset[std_col].values
                rel, rel_std = _relative_with_error(means, stds)
                idx = subset["Type"].tolist().index(container)
                rel_vals.append(rel[idx])
                rel_err.append(rel_std[idx])

            ax.bar(
                x + (j - (len(containers) - 1) / 2) * bar_w,
                rel_vals,
                yerr=rel_err,
                capsize=3,
                width=bar_w,
                label=container,
                color=COLORS.get(container, "gray"),
                alpha=0.9,
            )

        ax.set_xticks(x)
        ax.set_xticklabels(sizes, rotation=45, fontsize=fp["tick"])
        ax.set_title(title, fontsize=fp["title"])
        ax.grid(axis="y", linestyle="--", alpha=0.3)

    for ax in axes:
        ax.set_ylabel("Relative Time (% of best)", fontsize=fp["label"])
    axes[-1].set_xlabel("Benchmark Result Size", fontsize=fp["label"])
    fig.suptitle("Queue benchmarks", fontsize=fp["suptitle"])
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="upper center", ncol=len(containers), frameon=False, fontsize=fp["legend"])
    fig.tight_layout(rect=(0, 0, 1, 0.96))
    fig.savefig(out_path, dpi=220)
    plt.close(fig)
    print(f"Saved visualization: {out_path}")


def main():
    # User-requested scaling:
    # User-requested 20% reduction from 2x => 1.6x on all three figures
    font_multipliers = {
        "benchmark_results_list.csv": 1.6,
        "benchmark_results_deque.csv": 1.6,
        "benchmark_results_queue.csv": 1.6,
    }

    mappings = [
        ("benchmark_results_list.csv", "benchmark_results_list.png"),
        ("benchmark_results_deque.csv", "benchmark_results_deque.png"),
        ("benchmark_results_queue.csv", "benchmark_results_queue.png"),
    ]

    for src, dst in mappings:
        if not os.path.exists(src):
            print(f"Skipping missing file: {src}")
            continue

        df = pd.read_csv(src)
        font_mult = font_multipliers.get(src, 1.0)
        if "TimeMeanMs" in df.columns:
            plot_two_col_schema(src, dst, font_mult=font_mult)
        elif "PushHeavyMeanMs" in df.columns:
            plot_queue_schema(src, dst, font_mult=font_mult)
        else:
            print(f"Unknown schema, skipping: {src}")


if __name__ == "__main__":
    main()
