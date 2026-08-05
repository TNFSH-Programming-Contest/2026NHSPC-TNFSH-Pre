#include "Cake_4.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int ITERATIONS = 24;
int decodedN;
std::vector<int> decodedCuts;

std::size_t id(int n, int l, int r) {
    return static_cast<std::size_t>(l) * n + r;
}

std::uint64_t randomStep(std::uint64_t& state) {
    state ^= state << 7;
    state ^= state >> 9;
    state ^= state << 8;
    return state;
}

std::vector<int> anneal(const std::vector<std::vector<long long>>& w) {
    const int n = static_cast<int>(w.size());
    std::vector<long long> dp(static_cast<std::size_t>(n) * n);
    std::vector<int> opt(static_cast<std::size_t>(n) * n);
    for (int i = 0; i < n; ++i) opt[id(n, i, i)] = i;

    for (int length = 2; length <= n; ++length) {
        for (int l = 0; l + length <= n; ++l) {
            const int r = l + length - 1;
            const int low = std::max(opt[id(n, l, r - 1)], l);
            int high = std::min(opt[id(n, l + 1, r)], r - 1);
            if (high < low) high = low;

            const auto value = [&](int k) {
                return dp[id(n, l, k)] + dp[id(n, k + 1, r)];
            };
            int current = (low + high) / 2;
            long long currentValue = value(current);
            int bestCut = current;
            long long bestValue = currentValue;
            std::uint64_t state =
                0x67678787ULL ^ (static_cast<std::uint64_t>(l) << 32) ^ r;
            long double temperature =
                std::max<long double>(1.0L, std::fabs(currentValue) / 4.0L);

            for (int iteration = 0; iteration < ITERATIONS; ++iteration) {
                const int radius = std::max(
                    1, (high - low + 1) * (ITERATIONS - iteration) /
                           ITERATIONS);
                const int offset = static_cast<int>(
                    randomStep(state) % (2ULL * radius + 1)) - radius;
                const int next = std::max(low, std::min(current + offset, high));
                const long long nextValue = value(next);
                const long double probability =
                    std::exp(static_cast<long double>(currentValue - nextValue) /
                             temperature);
                const long double roll =
                    (randomStep(state) >> 11) * (1.0L / (1ULL << 53));
                if (nextValue <= currentValue || roll < probability) {
                    current = next;
                    currentValue = nextValue;
                }
                if (nextValue < bestValue) {
                    bestValue = nextValue;
                    bestCut = next;
                }
                temperature *= 0.78L;
            }
            dp[id(n, l, r)] = bestValue + w[l][r];
            opt[id(n, l, r)] = bestCut;
        }
    }
    return opt;
}

std::string encodeCuts(int n, const std::vector<int>& cuts) {
    std::string result;
    for (int l = 0; l + 1 < n; ++l) {
        int previous = l;
        for (int r = l + 1; r < n; ++r) {
            const int current = cuts[id(n, l, r)];
            result.append(current - previous, '1');
            result.push_back('0');
            previous = current;
        }
    }
    return result;
}

}  // namespace

std::string encode(int n, std::vector<std::vector<long long>> w) {
    return encodeCuts(n, anneal(w));
}

void decode(int n, std::string s) {
    decodedN = n;
    decodedCuts.assign(static_cast<std::size_t>(n) * n, 0);
    std::size_t position = 0;
    for (int l = 0; l + 1 < n; ++l) {
        int current = l;
        for (int r = l + 1; r < n; ++r) {
            while (position < s.size() && s[position] == '1') {
                ++current;
                ++position;
            }
            if (position < s.size()) ++position;
            decodedCuts[id(n, l, r)] = current;
        }
    }
}

int query(int l, int r) {
    return decodedCuts[id(decodedN, l, r)];
}
