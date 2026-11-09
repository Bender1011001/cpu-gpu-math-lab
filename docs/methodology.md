# Benchmark methodology

## Question

Can direct binary64 division accelerate arrays of exact, variable-divisor `uint32_t` quotients on the available CPU? Does it survive comparison with simple native-loop improvements and an approximate reciprocal refined in binary64?

The exploratory decision rule was a median speedup of at least 1.10 in an initial run and a fresh-seed confirmation, with all required correctness checks passing. The released harness reports raw results rather than declaring every machine a success.

## Workloads

Each of four distributions is tested at 4,096; 1,048,576; and 8,388,608 elements:

| Distribution | Numerator | Divisor |
|---|---|---|
| `full` | Random 32-bit unsigned | Random 32-bit unsigned; zero replaced with one |
| `small_divisors` | Random 32-bit unsigned | 1–256 |
| `large_quotients` | Random 32-bit unsigned | 1–9 |
| `near_multiples` | A random value rounded down to a divisor multiple, then adjusted by −1, 0, or +1 within range | 1–65,536 |

The generator uses `std::mt19937_64`. A separate generator shuffles timing order. Timing order may vary between standard libraries because `std::shuffle` is implementation-dependent. Input fingerprints use FNV-style XOR/multiply steps on whole `uint32_t` values, **not** standard bytewise FNV-1a.

## Procedure

1. Allocate and generate inputs outside timing.
2. Run every available method and compare every output with unsigned 64-bit integer division.
3. Warm each method three times.
4. Run an odd number of interleaved, randomized rounds using `std::chrono::steady_clock`.
5. Store every timing and its median in JSON. Write the final file only if all cases passed correctness checking.

Measured work includes reading both input arrays, all arithmetic and conversions, and writing the output array. Allocation, input generation, correctness checking, and compilation are excluded. There is one thread; no Python/operator dispatch is in the timing. Kernels are in a separate translation unit and the provided build does not enable link-time optimization.

The results are kernel throughput measurements. They do not include a full application, CPU/GPU transfer, compilation amortization, or an energy measurement.

## Published environment

| Setting | Value |
|---|---|
| CPU | Intel Core i5-13600K |
| OS | Windows 11 Pro, build 22631 |
| Compiler | Clang 22.1.8, LLVM revision `ca7933e47d3a3451d81e72ac174dcb5aa28b59d1` |
| Kernel options | `-O3 -march=native -std=c++17 -fno-math-errno -ffp-contract=off` |
| Optional kernels | AVX2 and FMA enabled |
| Initial run | Seed 20261135; 7 rounds; 3 warmups |
| Confirmation | Seed 20261136; 11 rounds; 3 warmups |
| Thread placement | Unpinned desktop process; core/frequency/background load uncontrolled |

Explicit FMA intrinsics remain permitted with contraction disabled. Automatic vectorization is allowed for every method. The native loops were not deliberately scalarized; the observed compiled integer instructions are a property of this compiler and target. Inspect the assembly on your compiler before drawing conclusions about its transformations.

## Reading the chart

The README chart shows the four largest confirmation cases. “Best native” is the minimum median among `native`, `unrolled`, and `shortcut` for each case. “Binary64” is the straightforward `float64` loop, not the best selected floating-point variant. The chart also includes the reciprocal challenger. All other methods and array sizes remain in the raw files.

The explicit biased-conversion variant sometimes improves the direct loop slightly. It did not establish a repeated 10% improvement in the original research comparison, so we do not present it as a separate finding.

## Limits and useful follow-ups

Small arrays take only a few microseconds and are sensitive to timer granularity and dispatch overhead. Large cases showed meaningful variation between runs too. Medians and two seeds provide evidence, not confidence intervals or universal performance guarantees.

This release has one hardware performance result. CI establishes build and sampled correctness on its runners; it is not a performance benchmark. AMD, ARM, other Intel CPUs, compiler versions that automatically optimize integer division, and larger application contexts may behave differently.

Useful next comparisons include another CPU, another compiler, pinned-core repeats, and workloads with reusable divisors. Keep accuracy and included costs matched when adding them.
