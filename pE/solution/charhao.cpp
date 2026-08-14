#include <string>

namespace {
    int decoded_K[2005][2005];
}

void decode(int n, const std::string& S) {
    for (int i = 0; i < n; i++) {
        decoded_K[i][i] = i;
    }
    for (int l = 0; l < n - 1; l++) {
        decoded_K[l][l + 1] = l;
    }

    int ptr = 0;
    for (int L = 3; L <= n; ++L) {
        for (int l = 0; l <= n - L; ++l) {
            int r = l + L - 1;
            int min_val = decoded_K[l][r - 1];
            int max_val = decoded_K[l + 1][r];
            int range_size = max_val - min_val + 1;

            if (range_size == 1) {
                decoded_K[l][r] = min_val;
                continue;
            }

            int K_bits = 0;
            while ((1 << (K_bits + 1)) <= range_size) K_bits++;

            int V = 0;
            if ((1 << K_bits) == range_size) {
                for (int i = K_bits - 1; i >= 0; i--) {
                    if (S[ptr++] == '1') V |= (1 << i);
                }
            } else {
                int limit = (1 << (K_bits + 1)) - range_size;
                // 先讀取較短的 K_bits 位元
                for (int i = K_bits - 1; i >= 0; i--) {
                    if (S[ptr++] == '1') V |= (1 << i);
                }
                // 若超出 limit，則需要再補讀取一位元
                if (V >= limit) {
                    V = (V << 1);
                    if (S[ptr++] == '1') V |= 1;
                    V -= limit;
                }
            }
            decoded_K[l][r] = min_val + V;
        }
    }
}

int query(int l, int r) {
    return decoded_K[l][r];
}

#include <string>
#include <array>

namespace {
    int K[2005][2005];
    long long dp[2005][2005];
}

std::string encode(int n, const std::array<std::array<long long, 2000>, 2000>& w) {
    for (int i = 0; i < n; i++) {
        dp[i][i] = 0;
    }

    // O(n^2) 區間 DP (Knuth Optimization)
    for (int L = 2; L <= n; ++L) {
        for (int l = 0; l <= n - L; ++l) {
            int r = l + L - 1;
            if (L == 2) {
                dp[l][r] = w[l][r];
                K[l][r] = l;
            } else {
                long long min_cost = -1;
                int best_k = -1;
                // 利用 Knuth 優化大幅限縮範圍
                int opt_min = K[l][r - 1];
                int opt_max = K[l + 1][r];

                for (int k = opt_min; k <= opt_max; ++k) {
                    if (k >= r) break; // 切點必須嚴格小於 r
                    long long cost = dp[l][k] + dp[k + 1][r];
                    if (min_cost == -1 || cost < min_cost) {
                        min_cost = cost;
                        best_k = k;
                    }
                }
                dp[l][r] = w[l][r] + min_cost;
                K[l][r] = best_k;
            }
        }
    }

    // 進行 Truncated Binary Encoding
    std::string S = "";
    for (int L = 3; L <= n; ++L) {
        for (int l = 0; l <= n - L; ++l) {
            int r = l + L - 1;
            int min_val = K[l][r - 1];
            int max_val = K[l + 1][r];
            int range_size = max_val - min_val + 1;
            int V = K[l][r] - min_val;

            if (range_size == 1) continue; // 只有一個選項時不耗費位元

            int K_bits = 0;
            while ((1 << (K_bits + 1)) <= range_size) K_bits++;

            if ((1 << K_bits) == range_size) {
                for (int i = K_bits - 1; i >= 0; i--) {
                    S.push_back((V & (1 << i)) ? '1' : '0');
                }
            } else {
                int limit = (1 << (K_bits + 1)) - range_size;
                if (V < limit) {
                    for (int i = K_bits - 1; i >= 0; i--) {
                        S.push_back((V & (1 << i)) ? '1' : '0');
                    }
                } else {
                    int encoded_val = V + limit;
                    for (int i = K_bits; i >= 0; i--) {
                        S.push_back((encoded_val & (1 << i)) ? '1' : '0');
                    }
                }
            }
        }
    }
    return S;
}