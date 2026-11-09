# Contributing

Reproductions, stronger baselines, counterexamples, and well-scoped experiments are welcome.

## Reproduce a result

Build in Release mode, run CTest, then run the benchmark with an explicit CPU label. Include the raw JSON, OS, compiler version, build flags, and whether the process was pinned to a core. Preserve all timing samples. A slower result is just as useful as a faster one.

If reporting a correctness failure, include the exact input pair, expected and actual outputs, method, compiler flags, and rounding mode. Do not include private machine paths or unrelated files.

## Add an experiment

Start from [the experiment template](docs/experiment-template.md). State the input domain, accuracy contract, baselines, and success criterion before measuring. Aim for a first experiment under ten minutes. Check difficult inputs as well as ordinary random data.

Use independent references where practical, include all relevant setup costs, and compare equivalent outputs at matched accuracy. A promising result needs a fresh run and a stronger-baseline challenge. Save negative outcomes to avoid repeating them.

Do not replace established acceptance checks just to make a candidate pass. Explain any legitimate change in the contract and keep the old result distinguishable.

## Code and documentation

The current C++ library has no third-party dependencies. Follow `.clang-format`, keep generated builds out of Git, and run CTest after changing numerical code. Python reporting tools use the standard library; plotting additionally requires Matplotlib. Performance numbers in documentation must link to raw measurements and state their hardware scope.

Contributions are under the repository's MIT license. Credit relevant prior art and distinguish a new implementation or measurement from new mathematics.
