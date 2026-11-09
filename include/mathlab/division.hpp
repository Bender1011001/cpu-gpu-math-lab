#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace mathlab {
using kernel = void (*)(const std::uint32_t *, const std::uint32_t *, std::uint32_t *, std::size_t);
struct method {
    const char *name;
    kernel run;
    bool direct_float64;
    bool reciprocal;
};

// Preconditions: all divisors are nonzero; buffers do not overlap; binary64
// arithmetic uses round-to-nearest. Values match uint32 division exactly.
// Floating-point exception flags need not match integer division.
void divide_float64(const std::uint32_t *numerator, const std::uint32_t *divisor,
                    std::uint32_t *quotient, std::size_t count);
std::vector<method> methods();
bool has_refined_reciprocal();
} // namespace mathlab
