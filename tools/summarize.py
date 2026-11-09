"""Validate saved timing summaries and print ratios; requires only Python's stdlib."""

import argparse
import json
import math
import statistics
from pathlib import Path


def read_results(path):
    data = json.loads(Path(path).read_text(encoding="utf-8"))
    if data["schema_version"] != 1 or data["all_outputs_exact"] is not True:
        raise ValueError("unsupported schema or failed recorded correctness checks")
    if len(data["cases"]) != 12:
        raise ValueError("expected all 12 benchmark cases")
    for case in data["cases"]:
        for name, method in case["methods"].items():
            samples = method["samples_ms"]
            if len(samples) != data["rounds"] or not all(math.isfinite(x) and x > 0 for x in samples):
                raise ValueError(f"invalid timings: {name}")
            if not math.isclose(statistics.median(samples), method["median_ms"], rel_tol=1e-10):
                raise ValueError(f"median does not match raw timings: {name}")
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("results", type=Path)
    args = parser.parse_args()
    data = read_results(args.results)
    print(f'{data["cpu"]} | {data["compiler"]} | seed {data["seed"]}')
    print("elements  distribution       best_native_ms  float64_ms  ratio")
    for case in data["cases"]:
        times = {name: method["median_ms"] for name, method in case["methods"].items()}
        baseline = min(times[name] for name in ("native", "unrolled", "shortcut"))
        print(f'{case["elements"]:8d}  {case["distribution"]:18s}  '
              f'{baseline:14.6f}  {times["float64"]:10.6f}  {baseline / times["float64"]:.3f}x')
    print("Timing summaries verified. Recorded correctness status is not an independent replay.")


if __name__ == "__main__":
    main()
