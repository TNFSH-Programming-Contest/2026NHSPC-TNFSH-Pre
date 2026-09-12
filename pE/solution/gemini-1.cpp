#include <string>
#include <array>
#include <algorithm>
#include <vector>
#include <cstdint>

namespace {
    const long long INF = 4e18;
    long long dp[2000][2000];
    int K[2000][2000];

    struct Choice {
        int offset;
        int choices;
    };
}

std::string encode(int n, const std::array<std::array<long long, 2000>, 2000>& w) {
    for (int i = 0; i < n; ++i) {
        dp[i][i] = 0;
        K[i][i] = i;
    }

    std::vector<Choice> items;
    items.reserve(n * n / 2);

    for (int len = 2; len <= n; ++len) {
        for (int l = 0; l <= n - len; ++l) {
            int r = l + len - 1;
            dp[l][r] = INF;

            int opt_start = K[l][r - 1];
            int opt_end = (l + 1 <= r) ? K[l + 1][r] : r - 1;
            opt_end = std::min(opt_end, r - 1);

            int best_k = opt_start;
            for (int k = opt_start; k <= opt_end; ++k) {
                long long val = w[l][r] + dp[l][k] + dp[k + 1][r];
                if (val < dp[l][r]) {
                    dp[l][r] = val;
                    best_k = k;
                }
            }
            K[l][r] = best_k;

            int choices = opt_end - opt_start + 1;
            if (choices > 1) {
                items.push_back({K[l][r] - opt_start, choices});
            }
        }
    }

    std::string bitstream = "";
    bitstream.reserve(2000000);

    // Group choices = 3 into blocks of 5 (3^5 = 243 < 256)
    std::vector<int> tri_block;
    for (const auto& item : items) {
        if (item.choices == 3) {
            tri_block.push_back(item.offset);
            if (tri_block.size() == 5) {
                int val = tri_block[0] + tri_block[1]*3 + tri_block[2]*9 + tri_block[3]*27 + tri_block[4]*81;
                for (int b = 7; b >= 0; --b) {
                    bitstream += ((val >> b) & 1) ? '1' : '0';
                }
                tri_block.clear();
            }
        } else {
            // Flush remaining base-3 items if any before standard items
            if (!tri_block.empty()) {
                int val = 0, p = 1;
                for (int v : tri_block) {
                    val += v * p;
                    p *= 3;
                }
                int bits = 0;
                while ((1 << bits) < p) bits++;
                for (int b = bits - 1; b >= 0; --b) {
                    bitstream += ((val >> b) & 1) ? '1' : '0';
                }
                tri_block.clear();
            }

            int bits = 0;
            while ((1 << bits) < item.choices) bits++;
            for (int b = bits - 1; b >= 0; --b) {
                bitstream += ((item.offset >> b) & 1) ? '1' : '0';
            }
        }
    }

    // Flush remaining base-3 items at the end
    if (!tri_block.empty()) {
        int val = 0, p = 1;
        for (int v : tri_block) {
            val += v * p;
            p *= 3;
        }
        int bits = 0;
        while ((1 << bits) < p) bits++;
        for (int b = bits - 1; b >= 0; --b) {
            bitstream += ((val >> b) & 1) ? '1' : '0';
        }
    }

    return bitstream;
}

#include <string>
#include <algorithm>
#include <vector>
#include <cstdint>

namespace {
    int Kdecode[2000][2000];
}

void decode(int n, const std::string& S) {
    for (int i = 0; i < n; ++i) {
        Kdecode[i][i] = i;
    }

    int ptr = 0;
    std::vector<int> tri_queue;

    for (int len = 2; len <= n; ++len) {
        for (int l = 0; l <= n - len; ++l) {
            int r = l + len - 1;

            int opt_start = Kdecode[l][r - 1];
            int opt_end = (l + 1 <= r) ? Kdecode[l + 1][r] : r - 1;
            opt_end = std::min(opt_end, r - 1);

            int choices = opt_end - opt_start + 1;
            if (choices <= 1) {
                Kdecode[l][r] = opt_start;
                continue;
            }

            int offset = 0;
            if (choices == 3) {
                if (tri_queue.empty()) {
                    // Peek ahead to see if a full block of 5 exists or remainder
                    int val = 0;
                    for (int b = 0; b < 8; ++b) {
                        val = (val << 1) | (S[ptr++] - '0');
                    }
                    for (int k = 0; k < 5; ++k) {
                        tri_queue.push_back(val % 3);
                        val /= 3;
                    }
                }
                offset = tri_queue.front();
                tri_queue.erase(tri_queue.begin());
            } else {
                tri_queue.clear();
                int bits = 0;
                while ((1 << bits) < choices) bits++;
                for (int b = 0; b < bits; ++b) {
                    offset = (offset << 1) | (S[ptr++] - '0');
                }
                if (offset >= choices) offset = choices - 1;
            }

            Kdecode[l][r] = opt_start + offset;
        }
    }
}

int query(int l, int r) {
    return Kdecode[l][r];
}