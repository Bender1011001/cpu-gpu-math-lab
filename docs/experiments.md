# Public experiment ledger

This ledger covers the reproducible experiments shipped in this repository. Earlier exploratory CPU/GPU work is not included in this first release and does not support additional public performance claims here.

| Experiment | Result | Evidence |
|---|---|---|
| Exact variable `uint32_t` division through binary64 | Useful on the tested i5-13600K/Clang combination; confirmation large-array speedup 2.50–2.77× over best native loop | [Raw runs](../results/README.md), [proof](exact-division.md) |
| Approximate reciprocal plus two binary64 Newton steps and residual correction | Passed included correctness checks; slower than direct binary64 division on the published runs | `reciprocal_refined` entries in raw runs |
| Explicit AVX2 biased conversions around direct division | Small, workload-dependent differences; no separate general improvement claimed | `float64_biased` entries in raw runs |

The mathematical division transformation is established prior art. The value of this release is reproducibility and comparison. A speedup on this kernel does not imply that every program, CPU, or GPU becomes faster.

## Next questions

- Which compilers and CPUs already perform this transformation automatically?
- Where do reusable divisors change the winner?
- Can another numerical primitive survive equally strict correctness and baseline challenges?

Use the [template](experiment-template.md) to propose the next bounded experiment.
