#include "Cake_4.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace {

int decodedN;
std::vector<int> decodedCuts;

std::size_t id(int n, int l, int r) {
    return static_cast<std::size_t>(l) * n + r;
}

std::int32_t wrap(long long value) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(value));
}

std::vector<int> computeWrongCuts(
    const std::vector<std::vector<long long>>& w) {
    const int n = static_cast<int>(w.size());
    std::vector<std::int32_t> dp(static_cast<std::size_t>(n) * n);
    std::vector<int> opt(static_cast<std::size_t>(n) * n);
    for (int i = 0; i < n; ++i) opt[id(n, i, i)] = i;

    for (int length = 2; length <= n; ++length) {
        for (int l = 0; l + length <= n; ++l) {
            const int r = l + length - 1;
            const int low = std::max(opt[id(n, l, r - 1)], l);
            const int high = std::min(opt[id(n, l + 1, r)], r - 1);
            std::int32_t best = std::numeric_limits<std::int32_t>::max();
            int bestCut = low;
            for (int k = low; k <= high; ++k) {
                const std::int32_t candidate = wrap(
                    static_cast<long long>(dp[id(n, l, k)]) +
                    dp[id(n, k + 1, r)]);
                if (candidate < best) {
                    best = candidate;
                    bestCut = k;
                }
            }
            dp[id(n, l, r)] = wrap(
                static_cast<long long>(best) + w[l][r]);
            opt[id(n, l, r)] = bestCut;
        }
    }
    return opt;
}

}  // namespace

std::string encode(int n, std::vector<std::vector<long long>> w) {
    const std::vector<int> cuts = computeWrongCuts(w);
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

