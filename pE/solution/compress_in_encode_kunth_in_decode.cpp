#include <bits/stdc++.h>
using namespace std;

#pragma GCC optimize("Ofast,unroll-loops")

namespace {

constexpr uint32_t TOP  = 0xFFFFFFFFu;
constexpr uint32_t HALF = 0x80000000u;
constexpr uint32_t Q1   = 0x40000000u;
constexpr uint32_t Q3   = 0xC0000000u;

constexpr int MAXBIT = 53;
constexpr int CTX = 256;
constexpr int MAXCNT = 4096;


/* ============================================================
 * Adaptive binary model
 *
 * context = previous up to 8 encoded bits
 * phase   = which kind of bit we are encoding
 *
 * phase:
 *   0 : zero/nonzero
 *   1 : bit-length unary
 *   2..54 : payload bit position
 * ============================================================ */

struct Model {
    uint16_t c0[55][CTX];
    uint16_t c1[55][CTX];

    int context;

    void init() {
        memset(c0, 0, sizeof(c0));
        memset(c1, 0, sizeof(c1));

        for (int p = 0; p < 55; ++p) {
            for (int c = 0; c < CTX; ++c) {
                c0[p][c] = 1;
                c1[p][c] = 1;
            }
        }

        context = 0;
    }

    void get(
        int phase,
        uint32_t& a,
        uint32_t& b
    ) const {
        a = c0[phase][context];
        b = c1[phase][context];
    }

    void update(
        int phase,
        int bit
    ) {
        uint16_t& a = c0[phase][context];
        uint16_t& b = c1[phase][context];

        if (uint32_t(a) + uint32_t(b) >= MAXCNT) {
            a = uint16_t((a + 1) >> 1);
            b = uint16_t((b + 1) >> 1);

            if (!a) a = 1;
            if (!b) b = 1;
        }

        if (bit) ++b;
        else ++a;

        context = ((context << 1) | bit) & 255;
    }
};


/* ============================================================
 * Bit writer
 * ============================================================ */

struct BitWriter {
    string s;

    inline void put(int x) {
        s.push_back(char('0' + x));
    }
};


/* ============================================================
 * Bit reader
 * ============================================================ */

struct BitReader {
    const string& s;
    size_t p;

    BitReader(
        const string& _s,
        size_t _p
    ) : s(_s), p(_p) {}

    inline int get() {
        if (p >= s.size())
            return 0;

        return s[p++] - '0';
    }
};


/* ============================================================
 * Arithmetic encoder
 * ============================================================ */

struct ArithmeticEncoder {
    uint32_t low = 0;
    uint32_t high = TOP;
    uint32_t pending = 0;

    BitWriter out;
    Model model;

    ArithmeticEncoder() {
        model.init();
    }

    inline void output_bit(int bit) {
        out.put(bit);

        while (pending) {
            out.put(bit ^ 1);
            --pending;
        }
    }

    void encode_bit(
        int phase,
        int bit
    ) {
        uint32_t c0, c1;
        model.get(phase, c0, c1);

        uint64_t range =
            uint64_t(high) -
            uint64_t(low) + 1;

        uint32_t total = c0 + c1;

        uint32_t mid =
            low +
            uint32_t((range * c0) / total) -
            1;

        if (bit == 0)
            high = mid;
        else
            low = mid + 1;

        while (true) {

            if (high < HALF) {
                output_bit(0);
            }
            else if (low >= HALF) {
                output_bit(1);

                low -= HALF;
                high -= HALF;
            }
            else if (low >= Q1 && high < Q3) {
                ++pending;

                low -= Q1;
                high -= Q1;
            }
            else {
                break;
            }

            low <<= 1;
            high = (high << 1) | 1;
        }

        model.update(phase, bit);
    }

    string finish() {
        ++pending;

        if (low < Q1)
            output_bit(0);
        else
            output_bit(1);

        return out.s;
    }
};


/* ============================================================
 * Arithmetic decoder
 * ============================================================ */

struct ArithmeticDecoder {
    uint32_t low = 0;
    uint32_t high = TOP;
    uint32_t code = 0;

    BitReader in;
    Model model;

    ArithmeticDecoder(
        const string& s,
        size_t pos
    ) : in(s, pos) {

        model.init();

        for (int i = 0; i < 32; ++i)
            code =
                (code << 1) |
                in.get();
    }

    int decode_bit(
        int phase
    ) {
        uint32_t c0, c1;
        model.get(phase, c0, c1);

        uint64_t range =
            uint64_t(high) -
            uint64_t(low) + 1;

        uint32_t total = c0 + c1;

        uint32_t mid =
            low +
            uint32_t((range * c0) / total) -
            1;

        int bit;

        if (code <= mid) {
            bit = 0;
            high = mid;
        }
        else {
            bit = 1;
            low = mid + 1;
        }

        while (true) {

            if (high < HALF) {
                // nothing
            }
            else if (low >= HALF) {
                low -= HALF;
                high -= HALF;
                code -= HALF;
            }
            else if (low >= Q1 && high < Q3) {
                low -= Q1;
                high -= Q1;
                code -= Q1;
            }
            else {
                break;
            }

            low <<= 1;
            high = (high << 1) | 1;

            code =
                (code << 1) |
                in.get();
        }

        model.update(phase, bit);

        return bit;
    }
};


/* ============================================================
 * ZigZag
 *
 * signed -> unsigned
 * ============================================================ */

static inline uint64_t zigzag(
    long long x
) {
    if (x >= 0)
        return uint64_t(x) << 1;

    return uint64_t(-(x + 1)) * 2 + 1;
}


/* ============================================================
 * inverse ZigZag
 * ============================================================ */

static inline long long unzigzag(
    uint64_t x
) {
    if ((x & 1) == 0)
        return (long long)(x >> 1);

    return -(long long)((x >> 1) + 1);
}


/* ============================================================
 * Encode unsigned integer
 *
 * x = 0:
 *     0
 *
 * x > 0:
 *
 *     1
 *     unary(bit_length - 1)
 *     0
 *     payload
 *
 * Example:
 *
 * x = 13 = 1101
 *
 *     nonzero = 1
 *     length  = 4 -> 1110
 *     payload = 1101
 *
 * Arithmetic coder compresses all of these bits.
 * ============================================================ */

static void encode_uint(
    ArithmeticEncoder& ac,
    uint64_t x
) {
    if (x == 0) {
        ac.encode_bit(0, 0);
        return;
    }

    ac.encode_bit(0, 1);

    int len = 64 - __builtin_clzll(x);

    /*
     * len is in [1, 53].
     *
     * Encode len-1 ones followed by zero.
     */
    for (int i = 1; i < len; ++i)
        ac.encode_bit(1, 1);

    ac.encode_bit(1, 0);

    /*
     * payload
     */
    for (int b = len - 1; b >= 0; --b) {
        int bit = (x >> b) & 1;

        /*
         * phase depends on payload position.
         *
         * 2 = highest bit
         * ...
         */
        int phase = 2 + (len - 1 - b);

        ac.encode_bit(
            min(54, phase),
            bit
        );
    }
}


/* ============================================================
 * Decode unsigned integer
 * ============================================================ */

static uint64_t decode_uint(
    ArithmeticDecoder& ac
) {
    int nonzero =
        ac.decode_bit(0);

    if (!nonzero)
        return 0;

    int len = 1;

    while (ac.decode_bit(1))
        ++len;

    uint64_t x = 0;

    for (int i = 0; i < len; ++i) {

        int phase =
            min(54, 2 + i);

        int bit =
            ac.decode_bit(phase);

        x =
            (x << 1) |
            uint64_t(bit);
    }

    return x;
}


/* ============================================================
 * Global decoded data
 * ============================================================ */

vector<vector<int>> opt;

vector<long long> W;
vector<long long> DP;


/* ============================================================
 * Build compressed representation
 *
 * For each l:
 *
 * base[l] = W[l][l+1]
 *
 * e[l][r] =
 *      W[l][r] - W[l+1][r]
 *
 * delta[l][r] =
 *      e[l][r] - e[l][r-1]
 *
 * By the Monge/QI property:
 *
 *      delta >= 0
 *
 * We encode:
 *
 * 1. all bases, using signed differences
 * 2. all deltas
 *
 * ============================================================ */

static void encode_values(
    int n,
    const array<array<long long, 2000>, 2000>& w,
    ArithmeticEncoder& ac
) {
    /*
     * Encode base sequence.
     *
     * Order:
     *
     * l = n-2, n-3, ..., 0
     *
     * Since no monotonicity of the base sequence is required,
     * we only use first-order signed delta as a cheap prediction.
     */

    long long prev = 0;

    bool first = true;

    for (int l = n - 2; l >= 0; --l) {

        long long x =
            w[l][l + 1];

        long long d;

        if (first) {
            d = x;
            first = false;
        }
        else {
            d = x - prev;
        }

        encode_uint(
            ac,
            zigzag(d)
        );

        prev = x;
    }


    /*
     * Now encode Monge second differences.
     */
    for (int l = n - 2; l >= 0; --l) {

        long long e =
            w[l][l + 1];

        /*
         * e[l][l+1]
         */
        long long prev_e = e;

        /*
         * There is no delta for r=l+1.
         */

        for (int r = l + 2; r < n; ++r) {

            long long cur_e =
                w[l][r] -
                w[l + 1][r];

            long long delta =
                cur_e - prev_e;

            /*
             * The problem guarantees the required
             * Monge / quadrangle inequality, so:
             *
             * delta >= 0
             */
            encode_uint(
                ac,
                uint64_t(delta)
            );

            prev_e = cur_e;
        }
    }
}


/* ============================================================
 * Restore W
 * ============================================================ */

static void decode_values(
    int n,
    ArithmeticDecoder& ac
) {
    W.assign(
        size_t(n) * n,
        0
    );


    /*
     * --------------------------------------------------------
     * 1. Restore bases
     * --------------------------------------------------------
     */

    long long prev = 0;

    bool first = true;

    for (int l = n - 2; l >= 0; --l) {

        uint64_t z =
            decode_uint(ac);

        long long d =
            unzigzag(z);

        long long x;

        if (first) {
            x = d;
            first = false;
        }
        else {
            x = prev + d;
        }

        W[size_t(l) * n + (l + 1)] = x;

        prev = x;
    }


    /*
     * --------------------------------------------------------
     * 2. Restore all rows
     *
     * e[l][r] = W[l][r] - W[l+1][r]
     *
     * delta = e[l][r] - e[l][r-1]
     *
     * Hence:
     *
     * e[l][r] =
     *     e[l][r-1] + delta
     *
     * and:
     *
     * W[l][r] =
     *     W[l+1][r] + e[l][r]
     * --------------------------------------------------------
     */

    for (int l = n - 2; l >= 0; --l) {

        long long e =
            W[size_t(l) * n + (l + 1)];

        for (int r = l + 2; r < n; ++r) {

            uint64_t delta =
                decode_uint(ac);

            e += (long long)(delta);

            W[size_t(l) * n + r] =
                W[size_t(l + 1) * n + r]
                + e;
        }
    }
}


/* ============================================================
 * Knuth optimization
 * ============================================================ */

static void build_opt(
    int n
) {
    const long long INF =
        (1LL << 62);

    DP.assign(
        size_t(n) * n,
        INF
    );

    opt.assign(
        n,
        vector<int>(n, 0)
    );

    for (int i = 0; i < n; ++i) {
        DP[size_t(i) * n + i] = 0;
        opt[i][i] = i;
    }


    /*
     * DP:
     *
     * dp[l][r] =
     *      W[l][r]
     *      + min(
     *          dp[l][k] +
     *          dp[k+1][r]
     *        )
     *
     * Knuth:
     *
     * opt[l][r-1]gg
     *     <= opt[l][r]
     *     <= opt[l+1][r]
     */
    for (int len = 2; len <= n; ++len) {

        for (int l = 0;
             l + len - 1 < n;
             ++l) {

            int r =
                l + len - 1;

            int L =
                opt[l][r - 1];

            int R =
                opt[l + 1][r];

            L = max(L, l);
            R = min(R, r - 1);

            long long best =
                INF;

            int best_k = L;

            for (int k = L; k <= R; ++k) {

                long long left =
                    DP[size_t(l) * n + k];

                long long right =
                    DP[size_t(k + 1) * n + r];

                long long cur =
                    left +
                    right +
                    W[size_t(l) * n + r];

                if (cur < best) {
                    best = cur;
                    best_k = k;
                }
            }

            DP[size_t(l) * n + r] =
                best;

            opt[l][r] =
                best_k;
        }
    }
}

} // anonymous namespace


/* ============================================================
 * ENCODE
 *
 * IMPORTANT:
 * This function is intentionally outside anonymous namespace.
 * ============================================================ */

string encode(
    int n,
    const array<array<long long, 2000>, 2000>& w
) {
    /*
     * Header:
     *
     * 6 bits magic/version.
     *
     * 010101 = version 1
     *
     * decode can therefore immediately know which format
     * is being used.
     */

    string ans;
    ans.reserve(4000000);

    const char header[] =
        "010101";

    ans.append(header);


    /*
     * Arithmetic coding starts after the fixed header.
     */

    ArithmeticEncoder ac;

    encode_values(
        n,
        w,
        ac
    );

    ans += ac.finish();


    /*
     * Guaranteed binary alphabet:
     *
     * arithmetic coder emits only '0' / '1'.
     */

    return ans;
}


/* ============================================================
 * DECODE
 *
 * IMPORTANT:
 * This function is intentionally outside anonymous namespace.
 * ============================================================ */

void decode(
    int n,
    const string& S
) {
    /*
     * Header.
     */

    size_t pos = 6;

    /*
     * Decode complete W.
     */

    ArithmeticDecoder ac(
        S,
        pos
    );

    decode_values(
        n,
        ac
    );


    /*
     * W is now completely reconstructed.
     *
     * Only now calculate Knuth optimization.
     */

    build_opt(n);
}


/* ============================================================
 * QUERY
 *
 * IMPORTANT:
 * This function is intentionally outside anonymous namespace.
 * ============================================================ */

int query(
    int l,
    int r
) {
    return opt[l][r];
}