# Why the binary64 quotient is exact

Consider integers `0 <= n <= 2^32 - 1` and `1 <= d <= 2^32 - 1`.
We want the unsigned integer quotient `floor(n/d)`.

Assume ordinary IEEE 754 binary64 conversion and division, round to nearest, and a truncating conversion back to `uint32_t`. Do not compile this transformation with relaxed division or fast-math assumptions.

1. Both integer inputs are represented exactly in binary64, which has 53 bits of precision.
2. If the real quotient is an integer, it is at most `2^32 - 1`, so binary64 division returns it exactly.
3. Otherwise write `n = q*d + r`, with `1 <= r < d`. The real quotient lies between adjacent integers `q` and `q+1`. Its distance from either endpoint is at least `1/d`.
4. For a nonzero quotient `x = n/d`, correct round-to-nearest binary64 division has absolute error at most `x * 2^-53`. All possible nonzero quotients here are normal binary64 values. Since `n < 2^32`, this error is less than `2^-21 / d`, and therefore strictly less than `1/d`.
5. Rounding cannot reach either neighboring integer. Truncating the rounded result therefore returns `q`. The zero numerator case is exact as well.

The rounded result is within the conversion range of `uint32_t`. The simple loop in [`divide_float64`](../src/division.cpp) applies this argument independently to each pair.

## Implementation contract

- Every divisor must be nonzero.
- Numerator, divisor, and output buffers must not overlap.
- Floating-point division must satisfy the semantics above. The harness checks the rounding mode; the source rejects GCC/Clang's `__FAST_MATH__` setting. Compiler options outside those checks remain the caller's responsibility.
- Floating-point exception flags can change, unlike integer division. Applications that trap floating-point exceptions or depend on their flags need separate consideration.
- This argument is specific to 32-bit unsigned inputs converted to binary64. It does not establish a general 64-bit integer replacement.

The optional `float64_biased` kernel uses the same quotient proof. Its conversion helpers encode four unsigned integers by inserting them into the low bits of binary64 values near `2^52`; subtracting the bias gives exact binary64 integers. Packing reverses that construction after explicit truncation.

## The reciprocal challenger

`reciprocal_refined` starts with an approximate single-precision reciprocal, performs two Newton iterations in binary64, and corrects the estimated quotient using a fused multiply-add residual. It is included as a performance challenger. It passed the included sampled and structured tests, but this document does **not** prove its full input domain. It was slower than direct division on the published machine.

## Prior art

[Daniel Lemire, “Fast exact integer divisions using floating-point operations” (2017)](https://lemire.me/blog/2017/11/16/fast-exact-integer-divisions-using-floating-point-operations/) describes this transformation and benchmarks it. We make no novelty claim for the mathematics.

[libdivide](https://libdivide.com/) is a relevant alternative when divisors are reused enough to amortize preparation. This experiment changes the divisor per element and does not measure that different workload.
