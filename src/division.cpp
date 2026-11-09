#include "mathlab/division.hpp"
#include <limits>
#if defined(__AVX2__) && defined(__FMA__)
#include <immintrin.h>
#define MATHLAB_VECTOR_REFINEMENT 1
#endif
#if defined(__FAST_MATH__)
#error "This experiment requires ordinary IEEE arithmetic; disable fast-math."
#endif
#if defined(_MSC_VER)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

namespace mathlab {
static_assert(std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::digits == 53,
              "IEEE binary64 double is required");
using u32 = std::uint32_t;

void divide_float64(const u32 *RESTRICT a, const u32 *RESTRICT b, u32 *RESTRICT out,
                    std::size_t n) {
    for (std::size_t i = 0; i < n; ++i)
        out[i] = static_cast<u32>(static_cast<double>(a[i]) / static_cast<double>(b[i]));
}
static void native(const u32 *RESTRICT a, const u32 *RESTRICT b, u32 *RESTRICT out, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i)
        out[i] = a[i] / b[i];
}
static void unrolled(const u32 *RESTRICT a, const u32 *RESTRICT b, u32 *RESTRICT out,
                     std::size_t n) {
    std::size_t i = 0;
    for (; i + 3 < n; i += 4) {
        out[i] = a[i] / b[i];
        out[i + 1] = a[i + 1] / b[i + 1];
        out[i + 2] = a[i + 2] / b[i + 2];
        out[i + 3] = a[i + 3] / b[i + 3];
    }
    for (; i < n; ++i)
        out[i] = a[i] / b[i];
}
static void shortcut(const u32 *RESTRICT a, const u32 *RESTRICT b, u32 *RESTRICT out,
                     std::size_t n) {
    for (std::size_t i = 0; i < n; ++i)
        out[i] = a[i] < b[i] ? 0 : a[i] / b[i];
}

#if MATHLAB_VECTOR_REFINEMENT
static inline __m256d as_double(__m128i v) {
    __m256i bits =
        _mm256_or_si256(_mm256_cvtepu32_epi64(v), _mm256_set1_epi64x(0x4330000000000000LL));
    return _mm256_sub_pd(_mm256_castsi256_pd(bits), _mm256_set1_pd(4503599627370496.0));
}
static inline __m128i as_uint32(__m256d integral) {
    __m256i bits = _mm256_castpd_si256(_mm256_add_pd(integral, _mm256_set1_pd(4503599627370496.0)));
    bits = _mm256_permutevar8x32_epi32(bits, _mm256_setr_epi32(0, 2, 4, 6, 0, 0, 0, 0));
    return _mm256_castsi256_si128(bits);
}
static void biased(const u32 *RESTRICT a, const u32 *RESTRICT b, u32 *RESTRICT out, std::size_t n) {
    std::size_t i = 0;
    for (; i + 3 < n; i += 4) {
        __m256d x = as_double(_mm_loadu_si128(reinterpret_cast<const __m128i *>(a + i)));
        __m256d y = as_double(_mm_loadu_si128(reinterpret_cast<const __m128i *>(b + i)));
        __m256d q = _mm256_round_pd(_mm256_div_pd(x, y), _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
        _mm_storeu_si128(reinterpret_cast<__m128i *>(out + i), as_uint32(q));
    }
    for (; i < n; ++i)
        out[i] = static_cast<u32>(static_cast<double>(a[i]) / b[i]);
}
static void refined(const u32 *RESTRICT a, const u32 *RESTRICT b, u32 *RESTRICT out,
                    std::size_t n) {
    std::size_t i = 0;
    for (; i + 3 < n; i += 4) {
        __m256d x = as_double(_mm_loadu_si128(reinterpret_cast<const __m128i *>(a + i)));
        __m256d y = as_double(_mm_loadu_si128(reinterpret_cast<const __m128i *>(b + i)));
        __m256d r = _mm256_cvtps_pd(_mm_rcp_ps(_mm256_cvtpd_ps(y)));
        r = _mm256_mul_pd(r, _mm256_fnmadd_pd(y, r, _mm256_set1_pd(2.0)));
        r = _mm256_mul_pd(r, _mm256_fnmadd_pd(y, r, _mm256_set1_pd(2.0)));
        __m256d q = _mm256_round_pd(_mm256_mul_pd(x, r), _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
        __m256d rem = _mm256_fnmadd_pd(q, y, x);
        q = _mm256_add_pd(q, _mm256_and_pd(_mm256_cmp_pd(rem, y, _CMP_GE_OQ), _mm256_set1_pd(1.0)));
        q = _mm256_sub_pd(q, _mm256_and_pd(_mm256_cmp_pd(rem, _mm256_setzero_pd(), _CMP_LT_OQ),
                                           _mm256_set1_pd(1.0)));
        _mm_storeu_si128(reinterpret_cast<__m128i *>(out + i), as_uint32(q));
    }
    for (; i < n; ++i)
        out[i] = a[i] / b[i];
}
#endif

std::vector<method> methods() {
    std::vector<method> result = {{"native", native, false, false},
                                  {"unrolled", unrolled, false, false},
                                  {"shortcut", shortcut, false, false},
                                  {"float64", divide_float64, true, false}};
#if MATHLAB_VECTOR_REFINEMENT
    result.push_back({"float64_biased", biased, true, false});
    result.push_back({"reciprocal_refined", refined, false, true});
#endif
    return result;
}
bool has_refined_reciprocal() {
#if MATHLAB_VECTOR_REFINEMENT
    return true;
#else
    return false;
#endif
}
} // namespace mathlab
