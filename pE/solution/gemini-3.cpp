#include <array>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>

#pragma GCC optimize("O3")

namespace {
    long long dp[2005][2005];
    int K_opt[2005][2005];
    const uint64_t L_bound = 1ULL << 31;
    const uint64_t S_total = 16384;      // 2^14
    const uint64_t q_val = (1ULL << 32) / S_total; // 2^18

    int get_bucket(int N) {
        if (N <= 6) return N - 2;
        if (N <= 8) return 5;
        if (N <= 12) return 6;
        if (N <= 20) return 7;
        if (N <= 40) return 8;
        if (N <= 80) return 9;
        if (N <= 200) return 10;
        if (N <= 500) return 11;
        return 12;
    }

    int get_num_cats_b(int b) {
        if (b <= 4) return b + 2;
        return 7;
    }

    int get_num_cats(int N) {
        if (N <= 6) return N;
        return 7;
    }

    int get_cat(int N, int val) {
        if (N <= 6) return val;
        if (val == 0) return 0;
        if (val == 1) return 1;
        if (val == 2) return 2;
        if (val == N - 3) return 3;
        if (val == N - 2) return 4;
        if (val == N - 1) return 5;
        return 6;
    }

    void get_W_cumW(int N, int val, const int* C, uint64_t &W, uint64_t &cum_W) {
        int num_cats = get_num_cats(N);
        uint64_t sum = 0;
        for (int i = 0; i < num_cats; i++) {
            if (i == 6) sum += C[i] * (uint64_t)(N - 6);
            else sum += C[i];
        }

        uint64_t w[7];
        for (int i = 0; i < num_cats; i++) {
            w[i] = (C[i] * 16384ULL) / sum;
            if (w[i] == 0) w[i] = 1;
        }

        if (num_cats == 7) {
            if ((N - 6) * w[6] >= 16384ULL - 6) {
                w[6] = (16384ULL - 6) / (N - 6);
                if (w[6] == 0) w[6] = 1;
            }
        }

        uint64_t exact_sum = 0;
        for (int i = 0; i < num_cats; i++) {
            if (i == 6) exact_sum += (N - 6) * w[6];
            else exact_sum += w[i];
        }

        if (exact_sum >= 16384) {
            for (int i = 0; i < num_cats; i++) w[i] = 1;
            exact_sum = (num_cats == 7) ? N : num_cats;
        }

        w[0] += 16384 - exact_sum;

        if (num_cats <= 6) {
            W = w[val];
            cum_W = 0;
            for (int i = 0; i < val; i++) cum_W += w[i];
        } else {
            if (val == 0) { W = w[0]; cum_W = 0; }
            else if (val == 1) { W = w[1]; cum_W = w[0]; }
            else if (val == 2) { W = w[2]; cum_W = w[0] + w[1]; }
            else if (val == N - 3) { W = w[3]; cum_W = w[0] + w[1] + w[2] + (uint64_t)(N - 6) * w[6]; }
            else if (val == N - 2) { W = w[4]; cum_W = w[0] + w[1] + w[2] + (uint64_t)(N - 6) * w[6] + w[3]; }
            else if (val == N - 1) { W = w[5]; cum_W = w[0] + w[1] + w[2] + (uint64_t)(N - 6) * w[6] + w[3] + w[4]; }
            else { W = w[6]; cum_W = w[0] + w[1] + w[2] + (uint64_t)(val - 3) * w[6]; }
        }
    }
}

std::string encode(int n, const std::array<std::array<long long, 2000>, 2000>& w) {
    for (int i = 0; i < n - 1; i++) {
        dp[i][i+1] = w[i][i+1];
        K_opt[i][i+1] = i;
    }

    std::vector<int> N_list;
    std::vector<int> val_list;
    N_list.reserve(n * n / 2);
    val_list.reserve(n * n / 2);

    for (int len = 2; len < n; len++) {
        for (int l = 0; l + len < n; l++) {
            int r = l + len;
            int L_k = K_opt[l][r-1];
            int R_k = K_opt[l+1][r];
            dp[l][r] = -1;
            int opt_k = L_k;

            for (int k = L_k; k <= R_k; k++) {
                if (k >= r) continue;
                long long cost = dp[l][k] + dp[k+1][r] + w[l][r];
                if (dp[l][r] == -1 || cost < dp[l][r]) {
                    dp[l][r] = cost;
                    opt_k = k;
                }
            }
            K_opt[l][r] = opt_k;

            int N = R_k - L_k + 1;
            if (N > 1) {
                N_list.push_back(N);
                val_list.push_back(opt_k - L_k);
            }
        }
    }

    int C[13][7];
    for (int b = 0; b < 13; b++)
        for (int j = 0; j < 7; j++) C[b][j] = 1;

    for (size_t i = 0; i < N_list.size(); i++) {
        int N = N_list[i], val = val_list[i];
        int b = get_bucket(N);
        int cat = get_cat(N, val);
        C[b][cat]++;
    }

    // 縮放各區間的計數器
    for (int b = 0; b < 13; b++) {
        int max_c = 0;
        int nc = get_num_cats_b(b);
        for (int j = 0; j < nc; j++) max_c = std::max(max_c, C[b][j]);
        while (max_c > 16383) {
            for (int j = 0; j < nc; j++) C[b][j] = (C[b][j] >> 1) | 1;
            max_c = 0;
            for (int j = 0; j < nc; j++) max_c = std::max(max_c, C[b][j]);
        }
    }

    uint64_t state = L_bound;
    std::string bits_str = "";
    bits_str.reserve(2000000);

    // 反向編碼 rANS
    for (int i = (int)N_list.size() - 1; i >= 0; i--) {
        int N = N_list[i], val = val_list[i];
        int b = get_bucket(N);

        uint64_t W, cum_W;
        get_W_cumW(N, val, C[b], W, cum_W);

        uint64_t max_x = W * q_val - 1;
        while (state > max_x) {
            bits_str += (state & 1) ? '1' : '0';
            state >>= 1;
        }
        state = (state / W) * S_total + (state % W) + cum_W;
    }

    // 輸出最終的 32 bits 狀態
    for (int i = 0; i < 32; i++) {
        bits_str += (state & 1) ? '1' : '0';
        state >>= 1;
    }

    // 寫入精緻化 Header 表頭
    auto emit_count = [&](int count) {
        for (int i = 0; i < 14; i++) {
            bits_str += (count & 1) ? '1' : '0';
            count >>= 1;
        }
    };

    for (int b = 0; b < 13; b++) {
        int nc = get_num_cats_b(b);
        for (int j = 0; j < nc; j++) {
            emit_count(C[b][j]);
        }
    }

    std::string S_out(bits_str.size(), '0');
    for (size_t i = 0; i < bits_str.size(); i++) {
        S_out[i] = bits_str[bits_str.size() - 1 - i];
    }
    return S_out;
}

#include <string>
#include <algorithm>

namespace {
    int K_dec[2005][2005];

    int get_val_and_W_cumW(int N, uint64_t slot, const int* C, uint64_t &W, uint64_t &cum_W) {
        int num_cats = get_num_cats(N);
        uint64_t sum = 0;
        for (int i = 0; i < num_cats; i++) {
            if (i == 6) sum += C[i] * (uint64_t)(N - 6);
            else sum += C[i];
        }

        uint64_t w[7];
        for (int i = 0; i < num_cats; i++) {
            w[i] = (C[i] * 16384ULL) / sum;
            if (w[i] == 0) w[i] = 1;
        }

        if (num_cats == 7) {
            if ((N - 6) * w[6] >= 16384ULL - 6) {
                w[6] = (16384ULL - 6) / (N - 6);
                if (w[6] == 0) w[6] = 1;
            }
        }

        uint64_t exact_sum = 0;
        for (int i = 0; i < num_cats; i++) {
            if (i == 6) exact_sum += (N - 6) * w[6];
            else exact_sum += w[i];
        }

        if (exact_sum >= 16384) {
            for (int i = 0; i < num_cats; i++) w[i] = 1;
            exact_sum = (num_cats == 7) ? N : num_cats;
        }

        w[0] += 16384 - exact_sum;

        if (num_cats <= 6) {
            uint64_t cw = 0;
            for (int i = 0; i < num_cats; i++) {
                if (slot < cw + w[i]) { W = w[i]; cum_W = cw; return i; }
                cw += w[i];
            }
            return num_cats - 1;
        } else {
            uint64_t cw = 0;
            if (slot < cw + w[0]) { W = w[0]; cum_W = cw; return 0; } cw += w[0];
            if (slot < cw + w[1]) { W = w[1]; cum_W = cw; return 1; } cw += w[1];
            if (slot < cw + w[2]) { W = w[2]; cum_W = cw; return 2; } cw += w[2];

            uint64_t mid_w = (N - 6) * w[6];
            if (slot < cw + mid_w) {
                uint64_t rem = slot - cw;
                int offset = (int)(rem / w[6]);
                W = w[6]; cum_W = cw + (uint64_t)offset * w[6];
                return 3 + offset;
            }
            cw += mid_w;

            if (slot < cw + w[3]) { W = w[3]; cum_W = cw; return N - 3; } cw += w[3];
            if (slot < cw + w[4]) { W = w[4]; cum_W = cw; return N - 2; } cw += w[4];
            if (slot < cw + w[5]) { W = w[5]; cum_W = cw; return N - 1; }

            return N - 1;
        }
    }
}

void decode(int n, const std::string& S) {
    for (int i = 0; i < n - 1; i++) {
        K_dec[i][i+1] = i;
    }

    size_t ptr = 0;

    auto read_count = [&]() {
        int C = 0;
        for (int i = 0; i < 14; i++) {
            char bit = (ptr < S.length()) ? S[ptr++] : '0';
            C = (C << 1) | (bit - '0');
        }
        return C;
    };

    int C[13][7];
    // 與 Encoder 的推入順序完美反轉 (因為 String 被反轉了)
    for (int b = 12; b >= 0; b--) {
        int nc = get_num_cats_b(b);
        for (int j = nc - 1; j >= 0; j--) {
            C[b][j] = read_count();
        }
    }

    uint64_t state = 0;
    for (int i = 0; i < 32; i++) {
        char bit = (ptr < S.length()) ? S[ptr++] : '0';
        state = (state << 1) | (bit - '0');
    }

    for (int len = 2; len < n; len++) {
        for (int l = 0; l + len < n; l++) {
            int r = l + len;
            int L_k = K_dec[l][r-1];
            int R_k = K_dec[l+1][r];
            int N = R_k - L_k + 1;

            if (N <= 1) {
                K_dec[l][r] = L_k;
                continue;
            }

            uint64_t slot = state % S_total;
            uint64_t W = 0, cum_W = 0;
            int b = get_bucket(N);
            int val = get_val_and_W_cumW(N, slot, C[b], W, cum_W);

            state = (state / S_total) * W + slot - cum_W;

            while (state < L_bound) {
                char bit = (ptr < S.length()) ? S[ptr++] : '0';
                state = (state << 1) | (bit - '0');
            }

            K_dec[l][r] = L_k + val;
        }
    }
}

int query(int l, int r) {
    return K_dec[l][r];
}