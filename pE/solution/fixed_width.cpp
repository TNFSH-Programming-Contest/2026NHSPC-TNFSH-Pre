#include "Cake_4.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace {

int decodedN;
std::vector<int> decodedCuts;

std::size_t id(int n, int l, int r) {
    return static_cast<std::size_t>(l) * n + r;
}

std::vector<int> computeCuts(
    const std::vector<std::vector<long long>>& w) {
    const int n = static_cast<int>(w.size());
    std::vector<long long> dp(static_cast<std::size_t>(n) * n);
    std::vector<int> opt(static_cast<std::size_t>(n) * n);
    for (int i = 0; i < n; ++i) {
        opt[id(n, i, i)] = i;
    }
    for (int length = 2; length <= n; ++length) {
        for (int l = 0; l + length <= n; ++l) {
            const int r = l + length - 1;
            const int low = std::max(opt[id(n, l, r - 1)], l);
            const int high = std::min(opt[id(n, l + 1, r)], r - 1);
            long long best = std::numeric_limits<long long>::max();
            int cut = low;
            for (int k = low; k <= high; ++k) {
                const long long candidate =
                    dp[id(n, l, k)] + dp[id(n, k + 1, r)];
                if (candidate < best) {
                    best = candidate;
                    cut = k;
                }
            }
            dp[id(n, l, r)] = best + w[l][r];
            opt[id(n, l, r)] = cut;
        }
    }
    return opt;
}

int bitCount(int n) {
    int bits = 0;
    while ((1 << bits) < n) ++bits;
    return bits;
}

}  // namespace

std::string encode(int n, std::vector<std::vector<long long>> w) {
    const std::vector<int> cuts = computeCuts(w);
    const int bits = bitCount(n);
    std::string result;
    result.reserve(static_cast<std::size_t>(n) * (n - 1) / 2 * bits);
    for (int l = 0; l + 1 < n; ++l) {
        for (int r = l + 1; r < n; ++r) {
            const int cut = cuts[id(n, l, r)];
            for (int bit = bits - 1; bit >= 0; --bit) {
                result.push_back((cut >> bit & 1) ? '1' : '0');
            }
        }
    }
    return result;
}

void decode(int n, std::string s) {
    decodedN = n;
    decodedCuts.assign(static_cast<std::size_t>(n) * n, 0);
    const int bits = bitCount(n);
    std::size_t position = 0;
    for (int l = 0; l + 1 < n; ++l) {
        for (int r = l + 1; r < n; ++r) {
            int cut = 0;
            for (int bit = 0; bit < bits; ++bit) {
                cut = cut * 2 + (s[position++] - '0');
            }
            decodedCuts[id(n, l, r)] = cut;
        }
    }
}

int query(int l, int r) {
    return decodedCuts[id(decodedN, l, r)];
}
