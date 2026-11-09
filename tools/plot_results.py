"""Render the README chart from raw confirmation results (requires Matplotlib)."""

import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from summarize import read_results


def main():
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path,
                        default=root / "results/cpu_division_i5_13600k_confirmation.json")
    parser.add_argument("--output", type=Path, default=root / "docs/assets/division-benchmark.svg")
    args = parser.parse_args()
    data = read_results(args.input)
    cases = [case for case in data["cases"] if case["elements"] == 8388608]
    titles = {"full": "Full-range inputs", "small_divisors": "Divisors 1–256",
              "large_quotients": "Divisors 1–9", "near_multiples": "Near integer multiples"}
    plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 11,
                         "svg.hashsalt": "cpu-gpu-math-lab-v1"})
    fig, axes = plt.subplots(2, 2, figsize=(12, 7.1), sharex=True)
    fig.set_facecolor("#f8fafc")
    fig.suptitle("Exact uint32 division", x=0.055, y=0.97, ha="left", fontsize=26,
                 fontweight="bold", color="#0f172a")
    fig.text(0.055, 0.90, "8,388,608 elements · Intel Core i5-13600K · Clang 22.1.8",
             color="#475569", fontsize=12)
    for ax, case in zip(axes.flat, cases):
        times = {name: method["median_ms"] for name, method in case["methods"].items()}
        baseline = min(times[name] for name in ("native", "unrolled", "shortcut"))
        values = [baseline, times["float64"], times["reciprocal_refined"]]
        ax.set_facecolor("#f8fafc")
        ax.barh([2, 1, 0], values, height=0.58, color=["#94a3b8", "#2563eb", "#d97706"], zorder=3)
        ax.set_yticks([2, 1, 0], ["Best native", "Binary64", "Refined reciprocal"])
        ax.tick_params(axis="both", length=0, colors="#475569")
        ax.set_title(titles[case["distribution"]], loc="left", fontsize=13, pad=16,
                     fontweight="bold", color="#0f172a")
        ax.set_xlim(0, 18.5)
        ax.set_xticks([0, 5, 10, 15])
        ax.grid(axis="x", color="#e2e8f0", zorder=0)
        for spine in ax.spines.values():
            spine.set_visible(False)
        for y, value in zip([2, 1, 0], values):
            ax.text(value + 0.2, y, f"{value:.2f}", va="center", color="#334155", fontsize=10)
        ax.text(0.99, 1.07, f'{baseline / times["float64"]:.2f}×', transform=ax.transAxes,
                ha="right", color="#2563eb", fontsize=15, fontweight="bold")
    fig.subplots_adjust(left=0.19, right=0.97, bottom=0.16, top=0.81, wspace=0.60, hspace=0.65)
    fig.text(0.055, 0.075, "Median milliseconds · lower is better · 11 interleaved rounds",
             color="#334155", fontsize=11)
    fig.text(0.055, 0.035, "One unpinned Windows desktop. Conversion and stores included; allocation excluded.",
             color="#64748b", fontsize=10)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(args.output, facecolor=fig.get_facecolor(), metadata={"Date": None})
    if args.output.suffix.lower() == ".svg":
        text = args.output.read_text(encoding="utf-8")
        args.output.write_text("\n".join(line.rstrip() for line in text.splitlines()) + "\n",
                               encoding="utf-8", newline="\n")
    fig.savefig(args.output.with_suffix(".png"), dpi=160, facecolor=fig.get_facecolor())
    print(args.output)


if __name__ == "__main__":
    main()
