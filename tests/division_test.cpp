#include "mathlab/division.hpp"
#include <cfenv>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

int main() {
    try {
        if (std::fegetround() != FE_TONEAREST)
            throw std::runtime_error("round-to-nearest required");
        using u32 = std::uint32_t;
        using u64 = std::uint64_t;
        std::vector<u32> a, b;
        std::mt19937_64 rng(20261135);
        constexpr u64 top = std::numeric_limits<u32>::max();
        auto add = [&](u64 n, u64 d) {
            if (n <= top && d > 0 && d <= top) {
                a.push_back(static_cast<u32>(n));
                b.push_back(static_cast<u32>(d));
            }
        };
        for (u64 d = 1; d < (1u << 20); ++d) {
            add(top, d);
            add((top / d) * d, d);
        }
        for (int bit = 0; bit <= 32; ++bit)
            for (int delta = -1; delta <= 1; ++delta) {
                const auto d = static_cast<std::int64_t>(u64{1} << bit) + delta;
                if (d <= 0 || static_cast<u64>(d) > top)
                    continue;
                for (u64 q :
                     {u64{0}, u64{1}, u64{2}, u64{17}, u64{65535}, top / static_cast<u64>(d)}) {
                    const auto n = q * static_cast<u64>(d);
                    add(n, static_cast<u64>(d));
                    if (n > 0)
                        add(n - 1, static_cast<u64>(d));
                    add(n + 1, static_cast<u64>(d));
                }
            }
        for (std::size_t i = 0; i < (1u << 20); ++i) {
            u32 d = static_cast<u32>(rng());
            if (d == 0)
                d = 1;
            add(static_cast<u32>(rng()), d);
            add(static_cast<u32>(rng()), 1 + rng() % 9);
        }
        std::vector<u32> out(a.size());
        std::uint64_t checked = 0;
        for (auto method : mathlab::methods()) {
            method.run(a.data(), b.data(), out.data(), a.size());
            for (std::size_t i = 0; i < a.size(); ++i) {
                const u64 reference = static_cast<u64>(a[i]) / static_cast<u64>(b[i]);
                if (out[i] != reference)
                    throw std::runtime_error(std::string(method.name) + " exactness mismatch");
                ++checked;
            }
            for (std::size_t n = 0; n <= 129; ++n) {
                for (std::size_t i = 0; i < n; ++i)
                    out[i] = ~static_cast<u32>(static_cast<u64>(a[i]) / b[i]);
                out[n] = 0xa5a5a5a5u;
                method.run(a.data(), b.data(), out.data(), n);
                for (std::size_t i = 0; i < n; ++i) {
                    if (out[i] != static_cast<u64>(a[i]) / b[i])
                        throw std::runtime_error("tail mismatch");
                    ++checked;
                }
                if (out[n] != 0xa5a5a5a5u)
                    throw std::runtime_error("output overrun");
            }
        }
        std::cout << "PASS: " << checked << " exact output checks; " << mathlab::methods().size()
                  << " methods; reciprocal=" << mathlab::has_refined_reciprocal() << "\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
