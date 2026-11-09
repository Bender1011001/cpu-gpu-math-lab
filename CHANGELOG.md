# Changelog

## 0.1.0

- Exact variable-divisor `uint32_t` division experiment in dependency-free C++17.
- Three native division baselines, a direct binary64 loop, and optional AVX2 conversion and reciprocal-refinement comparisons.
- Correctness checks, CMake/CTest builds, and Linux/Windows CI.
- Two local benchmark runs with every timing sample, a reproduction script, chart, mathematical proof, and explicit limitations.

This is an experimental first release. Published performance evidence comes from one CPU/compiler combination. The mathematical transformation is established prior art.
