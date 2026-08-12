#include "Cake_4.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace {
using Matrix = std::array<std::array<long long, 2000>, 2000>;

int decodedN;
std::vector<int> decodedCuts;

std::size_t index(int n, int l, int r) {
    return static_cast<std::size_t>(l) * n + r;
}

std::vector<int> computeCuts(const int n,
    const Matrix& w) {
    std::vector<long long> dp(static_cast<std::size_t>(n) * n);
    std::vector<int> opt(static_cast<std::size_t>(n) * n);
    for (int i = 0; i < n; ++i) {
        opt[index(n, i, i)] = i;
    }

    for (int length = 2; length <= n; ++length) {
        for (int l = 0; l + length <= n; ++l) {
            const int r = l + length - 1;
            const int low = std::max(opt[index(n, l, r - 1)], l);
            const int high = std::min(opt[index(n, l + 1, r)], r - 1);
            long long best = std::numeric_limits<long long>::max();
            int bestCut = low;
            for (int k = low; k <= high; ++k) {
                const long long candidate =
                    dp[index(n, l, k)] + dp[index(n, k + 1, r)];
                if (candidate < best) {
                    best = candidate;
                    bestCut = k;
                }
            }
            dp[index(n, l, r)] = best + w[l][r];
            opt[index(n, l, r)] = bestCut;
        }
    }
    return opt;
}

}  // namespace

std::string encode(int n, const Matrix& w) {
    const std::vector<int> cuts = computeCuts(n, w);
    std::string result;
    result.reserve(static_cast<std::size_t>(n - 1) * (n - 1));

    for (int l = 0; l + 1 < n; ++l) {
        int previous = l;
        for (int r = l + 1; r < n; ++r) {
            const int current = cuts[index(n, l, r)];
            result.append(current - previous, '1');
            result.push_back('0');
            previous = current;
        }
    }
    return result;
}

void decode(int n, const std::string& s) {
    decodedN = n;
    decodedCuts.assign(static_cast<std::size_t>(n) * n, 0);
    std::size_t position = 0;

    for (int l = 0; l + 1 < n; ++l) {
        int current = l;
        decodedCuts[index(n, l, l)] = l;
        for (int r = l + 1; r < n; ++r) {
            while (position < s.size() && s[position] == '1') {
                ++current;
                ++position;
            }
            if (position < s.size()) ++position;
            decodedCuts[index(n, l, r)] = current;
        }
    }
}

int query(int l, int r) {
    return decodedCuts[index(decodedN, l, r)];
}
