#include "mathlab/division.hpp"
#include <algorithm>
#include <cfenv>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using u32 = std::uint32_t;
using u64 = std::uint64_t;
static std::string quote(const std::string &s) {
    std::ostringstream o;
    o << '"';
    for (unsigned char c : s) {
        if (c == '"' || c == '\\')
            o << '\\' << c;
        else if (c < 32)
            o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<unsigned>(c)
              << std::dec;
        else
            o << c;
    }
    o << '"';
    return o.str();
}
static double median(std::vector<double> x) {
    std::sort(x.begin(), x.end());
    return x[x.size() / 2];
}
int main(int argc, char **argv) {
    try {
        std::string output = "results.json", cpu = "unspecified";
        u64 seed = 20261135;
        int rounds = 11;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--help") {
                std::cout << "mathlab_bench [--output FILE] [--seed N] [--rounds ODD_N] "
                             "[--cpu-label TEXT]\n";
                return 0;
            }
            if (i + 1 >= argc)
                throw std::runtime_error("missing argument value");
            if (arg == "--output")
                output = argv[++i];
            else if (arg == "--seed")
                seed = std::stoull(argv[++i]);
            else if (arg == "--rounds")
                rounds = std::stoi(argv[++i]);
            else if (arg == "--cpu-label")
                cpu = argv[++i];
            else
                throw std::runtime_error("unknown argument");
        }
        if (rounds < 3 || rounds % 2 == 0)
            throw std::runtime_error("use an odd number of rounds >=3");
        if (std::fegetround() != FE_TONEAREST)
            throw std::runtime_error("round-to-nearest required");
        const auto methods = mathlab::methods();
        std::mt19937_64 rng(seed), ordering(seed ^ 0x912f3a5u);
#if defined(__clang__)
        const std::string compiler = "Clang " __clang_version__;
#elif defined(__GNUC__)
        const std::string compiler = "GCC " __VERSION__;
#else
        const std::string compiler = "MSVC or other C++17 compiler";
#endif
        std::ostringstream json;
        json << std::setprecision(12) << "{\n  \"schema_version\":1,\n  \"seed\":" << seed
             << ",\n  \"rounds\":" << rounds << ",\n  \"warmups\":3,\n  \"cpu\":" << quote(cpu)
             << ",\n  \"compiler\":" << quote(compiler)
             << ",\n  \"native_tuning\":" << (MATHLAB_NATIVE_BUILD ? "true" : "false")
             << ",\n  \"reciprocal_available\":"
             << (mathlab::has_refined_reciprocal() ? "true" : "false")
             << ",\n  \"all_outputs_exact\":true,\n  \"cases\":[\n";
        bool first_case = true;
        u64 checked = 0;
        for (std::size_t n : {std::size_t{4096}, std::size_t{1} << 20, std::size_t{1} << 23})
            for (const std::string distribution :
                 {"full", "small_divisors", "large_quotients", "near_multiples"}) {
                std::vector<u32> a(n), b(n), out(n);
                u64 input_hash = 14695981039346656037ULL;
                for (std::size_t i = 0; i < n; ++i) {
                    a[i] = static_cast<u32>(rng());
                    b[i] = static_cast<u32>(rng());
                    if (b[i] == 0)
                        b[i] = 1;
                    if (distribution == "small_divisors")
                        b[i] = 1 + static_cast<u32>(rng() % 256);
                    if (distribution == "large_quotients")
                        b[i] = 1 + static_cast<u32>(rng() % 9);
                    if (distribution == "near_multiples") {
                        b[i] = 1 + static_cast<u32>(rng() % 65536);
                        u64 v = static_cast<u64>(a[i] / b[i]) * b[i];
                        const int offset = static_cast<int>(rng() % 3) - 1;
                        if (offset < 0 && v > 0)
                            --v;
                        if (offset > 0 && v < std::numeric_limits<u32>::max())
                            ++v;
                        a[i] = static_cast<u32>(v);
                    }
                    input_hash = (input_hash ^ a[i]) * 1099511628211ULL;
                    input_hash = (input_hash ^ b[i]) * 1099511628211ULL;
                }
                for (auto method : methods) {
                    method.run(a.data(), b.data(), out.data(), n);
                    for (std::size_t i = 0; i < n; ++i) {
                        if (out[i] != static_cast<u64>(a[i]) / b[i])
                            throw std::runtime_error(std::string(method.name) + " mismatch");
                        ++checked;
                    }
                    for (int warm = 0; warm < 3; ++warm)
                        method.run(a.data(), b.data(), out.data(), n);
                }
                std::vector<std::vector<double>> samples(methods.size());
                std::vector<std::size_t> order(methods.size());
                std::iota(order.begin(), order.end(), 0);
                for (int round = 0; round < rounds; ++round) {
                    std::shuffle(order.begin(), order.end(), ordering);
                    for (auto index : order) {
                        const auto begin = std::chrono::steady_clock::now();
                        methods[index].run(a.data(), b.data(), out.data(), n);
                        const auto end = std::chrono::steady_clock::now();
                        samples[index].push_back(
                            std::chrono::duration<double, std::milli>(end - begin).count());
                    }
                }
                if (!first_case)
                    json << ",\n";
                first_case = false;
                json << "    {\"elements\":" << n << ",\"distribution\":" << quote(distribution)
                     << ",\"input_hash_fnv1a_u32\":" << quote(std::to_string(input_hash))
                     << ",\"methods\":{\n";
                for (std::size_t j = 0; j < methods.size(); ++j) {
                    if (j)
                        json << ",\n";
                    json << "      " << quote(methods[j].name)
                         << ":{\"median_ms\":" << median(samples[j]) << ",\"samples_ms\":[";
                    for (std::size_t k = 0; k < samples[j].size(); ++k) {
                        if (k)
                            json << ',';
                        json << samples[j][k];
                    }
                    json << "]}";
                }
                json << "\n    }}";
                const auto direct = std::find_if(methods.begin(), methods.end(), [](auto m) {
                    return std::string(m.name) == "float64";
                });
                std::cout << n << ' ' << distribution << ": float64 "
                          << median(samples[static_cast<std::size_t>(direct - methods.begin())])
                          << " ms; all outputs exact\n";
            }
        json << "\n  ],\n  \"verified_output_count\":" << checked << "\n}\n";
        std::ofstream file(output);
        if (!file)
            throw std::runtime_error("cannot open output file");
        file << json.str();
        if (!file)
            throw std::runtime_error("failed writing output");
        std::cout << "Saved " << output << "; " << checked << " exact output checks\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
