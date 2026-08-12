#include "Cake_4.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace {
using Matrix = std::array<std::array<long long, 2000>, 2000>;

constexpr int DELTA_BITS = 4;
int decodedN;
std::vector<int> decodedCuts;

std::size_t id(int n, int l, int r) {
    return static_cast<std::size_t>(l) * n + r;
}

long long gcd(long long a, long long b) {
    while (b != 0) {
        const long long remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

std::vector<int> computeCuts(
    const int n, const std::vector<std::vector<long long>>& w) {
    std::vector<long long> dp(static_cast<std::size_t>(n) * n);
    std::vector<int> opt(static_cast<std::size_t>(n) * n);
    for (int i = 0; i < n; ++i) opt[id(n, i, i)] = i;

    for (int length = 2; length <= n; ++length) {
        for (int l = 0; l + length <= n; ++l) {
            const int r = l + length - 1;
            const int low = std::max(opt[id(n, l, r - 1)], l);
            const int high = std::min(opt[id(n, l + 1, r)], r - 1);
            long long best = std::numeric_limits<long long>::max();
            int bestCut = low;
            for (int k = low; k <= high; ++k) {
                const long long candidate =
                    dp[id(n, l, k)] + dp[id(n, k + 1, r)];
                if (candidate < best) {
                    best = candidate;
                    bestCut = k;
                }
            }
            dp[id(n, l, r)] = best + w[l][r];
            opt[id(n, l, r)] = bestCut;
        }
    }
    return opt;
}

}  // namespace

std::string encode(int n, const Matrix& w) {
    long long scale = 0;
    for (int l = 0; l < n; ++l) {
        for (int r = l + 1; r < n; ++r) {
            scale = gcd(scale, w[l][r]);
        }
    }

    std::string result;
    result.reserve(static_cast<std::size_t>(DELTA_BITS) * n * (n - 1) / 2);
    for (int r = 1; r < n; ++r) {
        for (int l = 0; l < r; ++l) {
            const long long outer =
                (w[l][r] - w[l][r - 1]) / scale;
            const long long inner =
                l + 1 == r
                    ? 1
                    : (w[l + 1][r] - w[l + 1][r - 1]) / scale;
            const int delta = static_cast<int>(outer - inner);
            for (int bit = DELTA_BITS - 1; bit >= 0; --bit) {
                result.push_back((delta >> bit & 1) ? '1' : '0');
            }
        }
    }
    return result;
}

void decode(int n, const std::string& s) {
    std::vector<std::vector<long long>> w(
        n, std::vector<long long>(n));
    std::size_t position = 0;
    for (int r = 1; r < n; ++r) {
        std::vector<int> delta(r);
        for (int l = 0; l < r; ++l) {
            for (int bit = 0; bit < DELTA_BITS; ++bit) {
                delta[l] = delta[l] * 2 + (s[position++] - '0');
            }
        }
        long long extensionCost = 1;
        for (int l = r - 1; l >= 0; --l) {
            extensionCost += delta[l];
            w[l][r] = w[l][r - 1] + extensionCost;
        }
    }

    decodedN = n;
    decodedCuts = computeCuts(n, w);
}

int query(int l, int r) {
    return decodedCuts[id(decodedN, l, r)];
}
