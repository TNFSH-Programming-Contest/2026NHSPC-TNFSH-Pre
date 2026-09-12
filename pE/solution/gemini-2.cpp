#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <cstdint>

namespace {
    struct BigInt {
        std::vector<uint32_t> limbs; // 以 2^32 為進制

        BigInt(uint32_t v = 0) {
            if (v > 0) limbs.push_back(v);
        }

        void clamp() {
            while (!limbs.empty() && limbs.back() == 0) limbs.pop_back();
        }

        void mul_add(uint32_t base, uint32_t add) {
            uint64_t carry = add;
            for (size_t i = 0; i < limbs.size(); ++i) {
                uint64_t prod = (uint64_t)limbs[i] * base + carry;
                limbs[i] = (uint32_t)(prod & 0xFFFFFFFF);
                carry = prod >> 32;
            }
            if (carry > 0) {
                limbs.push_back((uint32_t)carry);
            }
            clamp();
        }

        int bit_length() const {
            if (limbs.empty()) return 0;
            uint32_t top = limbs.back();
            int b = 32 - __builtin_clz(top);
            return (int)(limbs.size() - 1) * 32 + b;
        }

        void set_bit(int idx) {
            size_t limb_idx = idx / 32;
            int bit_pos = idx % 32;
            while (limbs.size() <= limb_idx) limbs.push_back(0);
            limbs[limb_idx] |= (1U << bit_pos);
        }

        static BigInt add(const BigInt& a, const BigInt& b) {
            BigInt res;
            uint64_t carry = 0;
            size_t n = std::max(a.limbs.size(), b.limbs.size());
            for (size_t i = 0; i < n || carry; ++i) {
                uint64_t v1 = (i < a.limbs.size()) ? a.limbs[i] : 0;
                uint64_t v2 = (i < b.limbs.size()) ? b.limbs[i] : 0;
                uint64_t s = v1 + v2 + carry;
                res.limbs.push_back((uint32_t)(s & 0xFFFFFFFF));
                carry = s >> 32;
            }
            res.clamp();
            return res;
        }

        static BigInt sub(const BigInt& a, const BigInt& b) {
            BigInt res;
            int64_t borrow = 0;
            for (size_t i = 0; i < a.limbs.size(); ++i) {
                int64_t v1 = a.limbs[i];
                int64_t v2 = (i < b.limbs.size()) ? b.limbs[i] : 0;
                int64_t diff = v1 - v2 - borrow;
                if (diff < 0) {
                    diff += (1LL << 32);
                    borrow = 1;
                } else {
                    borrow = 0;
                }
                res.limbs.push_back((uint32_t)diff);
            }
            res.clamp();
            return res;
        }

        static bool is_less(const BigInt& a, const BigInt& b) {
            if (a.limbs.size() != b.limbs.size())
                return a.limbs.size() < b.limbs.size();
            for (int i = (int)a.limbs.size() - 1; i >= 0; --i) {
                if (a.limbs[i] != b.limbs[i])
                    return a.limbs[i] < b.limbs[i];
            }
            return false;
        }

        static BigInt power_of_two(int k) {
            BigInt res;
            res.set_bit(k);
            return res;
        }
    };

    long long cost[2000][2000];
    int K[2000][2000];

    void encode_bigint_tb(std::string& S, const BigInt& V, const BigInt& M) {
        if (M.limbs.empty() || (M.limbs.size() == 1 && M.limbs[0] <= 1)) return;
        int k = M.bit_length() - 1;
        BigInt u = BigInt::sub(BigInt::power_of_two(k + 1), M);

        BigInt val_to_send;
        int bits;
        if (BigInt::is_less(V, u)) {
            val_to_send = V;
            bits = k;
        } else {
            val_to_send = BigInt::add(V, u);
            bits = k + 1;
        }

        for (int b = bits - 1; b >= 0; --b) {
            size_t limb_idx = b / 32;
            int bit_pos = b % 32;
            bool bit_val = false;
            if (limb_idx < val_to_send.limbs.size()) {
                bit_val = (val_to_send.limbs[limb_idx] >> bit_pos) & 1;
            }
            S.push_back(bit_val ? '1' : '0');
        }
    }
}

std::string encode(int n, const std::array<std::array<long long, 2000>, 2000>& w) {
    std::string S = "";

    for (int i = 0; i < n; ++i) {
        cost[i][i] = 0;
        K[i][i] = i;
    }
    for (int i = 0; i < n - 1; ++i) {
        cost[i][i + 1] = w[i][i + 1];
        K[i][i + 1] = i;
    }

    for (int len = 3; len <= n; ++len) {
        BigInt V(0), M(1);
        for (int i = 0; i <= n - len; ++i) {
            int j = i + len - 1;
            int L = K[i][j - 1];
            int R = K[i + 1][j];

            long long min_c = -1;
            int best_k = L;

            for (int k = L; k <= R; ++k) {
                long long c = cost[i][k] + cost[k + 1][j] + w[i][j];
                if (min_c == -1 || c < min_c) {
                    min_c = c;
                    best_k = k;
                }
            }

            cost[i][j] = min_c;
            K[i][j] = best_k;

            uint32_t d = R - L;
            uint32_t v = best_k - L;

            V.mul_add(d + 1, v);
            M.mul_add(d + 1, 0);
        }

        encode_bigint_tb(S, V, M);
    }

    return S;
}

#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>

namespace {
    struct DecodeBigInt {
        std::vector<uint32_t> limbs;

        DecodeBigInt(uint32_t v = 0) {
            if (v > 0) limbs.push_back(v);
        }

        void clamp() {
            while (!limbs.empty() && limbs.back() == 0) limbs.pop_back();
        }

        void mul_add(uint32_t base, uint32_t add) {
            uint64_t carry = add;
            for (size_t i = 0; i < limbs.size(); ++i) {
                uint64_t prod = (uint64_t)limbs[i] * base + carry;
                limbs[i] = (uint32_t)(prod & 0xFFFFFFFF);
                carry = prod >> 32;
            }
            if (carry > 0) {
                limbs.push_back((uint32_t)carry);
            }
            clamp();
        }

        uint32_t div_mod(uint32_t base) {
            uint64_t rem = 0;
            for (int i = (int)limbs.size() - 1; i >= 0; --i) {
                uint64_t cur = (rem << 32) | limbs[i];
                limbs[i] = (uint32_t)(cur / base);
                rem = cur % base;
            }
            clamp();
            return (uint32_t)rem;
        }

        int bit_length() const {
            if (limbs.empty()) return 0;
            uint32_t top = limbs.back();
            int b = 32 - __builtin_clz(top);
            return (int)(limbs.size() - 1) * 32 + b;
        }

        void set_bit(int idx) {
            size_t limb_idx = idx / 32;
            int bit_pos = idx % 32;
            while (limbs.size() <= limb_idx) limbs.push_back(0);
            limbs[limb_idx] |= (1U << bit_pos);
        }

        static DecodeBigInt sub(const DecodeBigInt& a, const DecodeBigInt& b) {
            DecodeBigInt res;
            int64_t borrow = 0;
            for (size_t i = 0; i < a.limbs.size(); ++i) {
                int64_t v1 = a.limbs[i];
                int64_t v2 = (i < b.limbs.size()) ? b.limbs[i] : 0;
                int64_t diff = v1 - v2 - borrow;
                if (diff < 0) {
                    diff += (1LL << 32);
                    borrow = 1;
                } else {
                    borrow = 0;
                }
                res.limbs.push_back((uint32_t)diff);
            }
            res.clamp();
            return res;
        }

        static bool is_less(const DecodeBigInt& a, const DecodeBigInt& b) {
            if (a.limbs.size() != b.limbs.size())
                return a.limbs.size() < b.limbs.size();
            for (int i = (int)a.limbs.size() - 1; i >= 0; --i) {
                if (a.limbs[i] != b.limbs[i])
                    return a.limbs[i] < b.limbs[i];
            }
            return false;
        }

        static DecodeBigInt power_of_two(int k) {
            DecodeBigInt res;
            res.set_bit(k);
            return res;
        }
    };

    int Kdecode[2000][2000];

    DecodeBigInt decode_bigint_tb(const std::string& S, int& ptr, const DecodeBigInt& M) {
        if (M.limbs.empty() || (M.limbs.size() == 1 && M.limbs[0] <= 1)) return DecodeBigInt(0);
        int k = M.bit_length() - 1;
        DecodeBigInt u = DecodeBigInt::sub(DecodeBigInt::power_of_two(k + 1), M);

        DecodeBigInt W(0);
        for (int i = 0; i < k; ++i) {
            uint32_t bit_val = S[ptr++] - '0';
            W.mul_add(2, bit_val);
        }

        if (DecodeBigInt::is_less(W, u)) {
            return W;
        } else {
            uint32_t extra_bit = S[ptr++] - '0';
            W.mul_add(2, extra_bit);
            return DecodeBigInt::sub(W, u);
        }
    }
}

void decode(int n, const std::string& S) {
    for (int i = 0; i < n; ++i) {
        Kdecode[i][i] = i;
    }
    for (int i = 0; i < n - 1; ++i) {
        Kdecode[i][i + 1] = i;
    }

    int ptr = 0;
    for (int len = 3; len <= n; ++len) {
        int m = n - len;
        std::vector<uint32_t> bases(m + 1);
        std::vector<int> L_list(m + 1);
        DecodeBigInt M(1);

        for (int i = 0; i <= m; ++i) {
            int j = i + len - 1;
            int L = Kdecode[i][j - 1];
            int R = Kdecode[i + 1][j];
            uint32_t d = R - L;

            bases[i] = d + 1;
            L_list[i] = L;
            M.mul_add(d + 1, 0);
        }

        DecodeBigInt V = decode_bigint_tb(S, ptr, M);

        for (int i = m; i >= 0; --i) {
            int j = i + len - 1;
            uint32_t v = V.div_mod(bases[i]);
            Kdecode[i][j] = L_list[i] + (int)v;
        }
    }
}

int query(int l, int r) {
    return Kdecode[l][r];
}