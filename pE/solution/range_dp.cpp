#include "Cake_4.h"

#include <cstddef>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

int decodedN;
std::vector<int> decodedCuts;

std::size_t id(int n, int l, int r) {
    return static_cast<std::size_t>(l) * n + r;
}

}  // namespace

std::string encode(int n, std::vector<std::vector<long long>> w) {
    std::vector<long long> dp(static_cast<std::size_t>(n) * n);
    std::vector<int> opt(static_cast<std::size_t>(n) * n);
    for (int i = 0; i < n; ++i) {
        opt[id(n, i, i)] = i;
    }

    for (int length = 2; length <= n; ++length) {
        for (int l = 0; l + length <= n; ++l) {
            const int r = l + length - 1;
            std::multiset<std::pair<long long, int>> candidates;
            for (int k = l; k < r; ++k) {
                candidates.emplace(
                    dp[id(n, l, k)] + dp[id(n, k + 1, r)], k);
            }
            dp[id(n, l, r)] =
                candidates.begin()->first + w[l][r];
            opt[id(n, l, r)] = candidates.begin()->second;
        }
    }

    std::string result;
    for (int l = 0; l + 1 < n; ++l) {
        int previous = l;
        for (int r = l + 1; r < n; ++r) {
            const int current = opt[id(n, l, r)];
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


