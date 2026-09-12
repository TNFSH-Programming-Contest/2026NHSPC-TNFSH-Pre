#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using ull = unsigned long long;

#pragma GCC optimize("Ofast")

constexpr int MOD = 1145141;
constexpr int LIM = 31623;

constexpr int REM[8] = {
    1, 7, 11, 13, 17, 19, 23, 29
};

/*
 * compile-time presieve:
 * 2, 3, 5 由 wheel 排除
 * 再排除 7, 11, 13, 17, 19
 */
constexpr int PREP[5] = {
    7, 11, 13, 17, 19
};

constexpr int PRE_WHEEL =
    2 * 3 * 5 * 7 * 11 * 13 * 17 * 19;

// 9699690 / 30 = 323323 bytes
constexpr int PRE_BLOCKS =
    PRE_WHEEL / 30;

/*
 * 一塊 constexpr 只做 65536 bytes。
 *
 * 323323 =
 *   65536 * 4 + 61179
 */
constexpr int CH = 1 << 16;

/*
 * runtime segment 不必跟 presieve period 相同。
 */
constexpr int SEG_BLOCKS = 1 << 15;

constexpr int rem_id(int x) {
    return
        x == 1  ? 0 :
        x == 7  ? 1 :
        x == 11 ? 2 :
        x == 13 ? 3 :
        x == 17 ? 4 :
        x == 19 ? 5 :
        x == 23 ? 6 : 7;
}

/* ============================================================
 * Compile-time Eratosthenes <= sqrt(1e9)
 * ============================================================ */

struct PrimeTable {
    array<ull, (LIM + 64) / 64> bit{};
    array<uint16_t, 4000> prime{};
    int cnt = 0;

    constexpr PrimeTable() {
        for(int i = 0; i < (int)bit.size(); ++i)
            bit[i] = ~0ULL;

        bit[0] &= ~3ULL;

        for(int i = 2; i * i <= LIM; ++i) {
            if((bit[i >> 6] >> (i & 63)) & 1ULL) {
                for(int j = i * i; j <= LIM; j += i)
                    bit[j >> 6] &=
                        ~(1ULL << (j & 63));
            }
        }

        /*
         * 2..19 都會另外處理。
         */
        for(int i = 23; i <= LIM; ++i) {
            if((bit[i >> 6] >> (i & 63)) & 1ULL)
                prime[cnt++] = i;
        }
    }
};

constexpr PrimeTable PT;

/* ============================================================
 * Compile-time presieve chunks
 * ============================================================ */

template<int OFF, int LEN>
struct PresieveChunk {
    array<uint8_t, LEN> a{};

    constexpr PresieveChunk() {
        for(int i = 0; i < LEN; ++i)
            a[i] = 0xff;

        for(int z = 0; z < 5; ++z) {
            const int p = PREP[z];

            for(int bit = 0; bit < 8; ++bit) {
                /*
                 * 找這個 chunk 中第一個：
                 *
                 * 30*b + REM[bit] ≡ 0 (mod p)
                 */
                int b = OFF;

                while((30 * b + REM[bit]) % p)
                    ++b;

                for(
                    ;
                    b < OFF + LEN;
                    b += p
                ) {
                    a[b - OFF] &=
                        uint8_t(~(1u << bit));
                }
            }
        }
    }
};

constexpr PresieveChunk<
    0,
    CH
> P0;

constexpr PresieveChunk<
    CH,
    CH
> P1;

constexpr PresieveChunk<
    2 * CH,
    CH
> P2;

constexpr PresieveChunk<
    3 * CH,
    CH
> P3;

constexpr PresieveChunk<
    4 * CH,
    PRE_BLOCKS - 4 * CH
> P4;

/* ============================================================
 * Compile-time initial strike streams
 * ============================================================ */

struct NxtTable {
    array<array<uint32_t, 8>, 4000> a{};

    constexpr NxtTable() {
        for(int z = 0; z < PT.cnt; ++z) {
            const int p = PT.prime[z];
            const int pr = p % 30;

            for(int t = 0; t < 8; ++t) {
                const int r = REM[t];

                int m = p;

                int d =
                    r - (m % 30);

                if(d < 0)
                    d += 30;

                m += d;

                const ll x =
                    1LL * p * m;

                const int bit =
                    rem_id(pr * r % 30);

                a[z][bit] =
                    uint32_t(x / 30);
            }
        }
    }
};

constexpr NxtTable NT;

/* ============================================================ */

alignas(64)
uint8_t seg[SEG_BLOCKS];

alignas(64)
uint32_t nxt[4000][8];

/*
 * 取得 presieve chunk。
 */
inline const uint8_t* part_ptr(int id) {
    switch(id) {
        case 0:
            return P0.a.data();

        case 1:
            return P1.a.data();

        case 2:
            return P2.a.data();

        case 3:
            return P3.a.data();

        default:
            return P4.a.data();
    }
}

inline int part_size(int id) {
    return
        id < 4
        ? CH
        : PRE_BLOCKS - 4 * CH;
}

/*
 * 從週期 PRE_BLOCKS 的 compile-time pattern
 * 複製目前 segment。
 */
inline void fill_presieve(
    uint32_t baseBlock,
    int blocks
) {
    int off =
        baseBlock % PRE_BLOCKS;

    int done = 0;

    while(done < blocks) {
        int id =
            off / CH;

        int local =
            off - id * CH;

        int take =
            min(
                blocks - done,
                part_size(id) - local
            );

        memcpy(
            seg + done,
            part_ptr(id) + local,
            take
        );

        done += take;
        off += take;

        if(off == PRE_BLOCKS)
            off = 0;
    }
}

/* ============================================================ */

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    int ans = 1;

    auto calc = [&](ll p) {
        int e = 0;

        for(ll x = p; x <= n; ) {
            e += n / x;

            if(x > n / p)
                break;

            x *= p;
        }

        ans =
            1LL * ans * (e + 1) % MOD;
    };

    /*
     * 這些 prime 被 wheel/presieve 排除了，
     * 自己計算。
     */
    for(int p : {
        2, 3, 5,
        7, 11, 13, 17, 19
    }) {
        if(p <= n)
            calc(p);
    }

    int root =
        sqrt((double)n);

    while(
        1LL *
        (root + 1) *
        (root + 1)
        <= n
    )
        ++root;

    while(
        1LL * root * root > n
    )
        --root;

    /*
     * strike stream 起點也是 compile-time。
     */
    memcpy(
        nxt,
        NT.a.data(),
        sizeof(nxt)
    );

    /*
     * p > sqrt(n)：
     *
     * v_p(n!) = floor(n/p)
     */
    int q = 0;
    ll q_right = 0;

    ull mul = 0;
    ull limit = 0;
    ull acc = 0;

    bool large = false;

    for(
        uint32_t baseBlock = 0;
        30ULL * baseBlock <= (ull)n;
        baseBlock += SEG_BLOCKS
    ) {
        const ll base =
            30LL * baseBlock;

        const ll R =
            min<ll>(
                n,
                base +
                30LL * SEG_BLOCKS -
                1
            );

        const int blocks =
            int((R - base) / 30) + 1;

        /*
         * 直接把已經排掉
         * 2,3,5,7,11,13,17,19
         * 的 pattern 複製進來。
         */
        fill_presieve(
            baseBlock,
            blocks
        );

        if(baseBlock == 0)
            seg[0] &= ~1u;

        const uint32_t endBlock =
            baseBlock + blocks;

        /*
         * Runtime sieve 從 p=23 開始。
         */
        for(int z = 0; z < PT.cnt; ++z) {
            const uint32_t p =
                PT.prime[z];

            if(
                1ULL * p * p >
                (ull)R
            )
                break;

            for(
                int bit = 0;
                bit < 8;
                ++bit
            ) {
                uint32_t b =
                    nxt[z][bit];

                const uint8_t mask =
                    uint8_t(
                        ~(1u << bit)
                    );

                for(
                    ;
                    b < endBlock;
                    b += p
                ) {
                    seg[
                        b - baseBlock
                    ] &= mask;
                }

                nxt[z][bit] = b;
            }
        }

        /*
         * Enumerate surviving primes.
         */
        for(int b = 0; b < blocks; ++b) {
            unsigned v =
                seg[b];

            while(v) {
                const int bit =
                    __builtin_ctz(v);

                v &= v - 1;

                const ll p =
                    base +
                    30LL * b +
                    REM[bit];

                if(
                    p < 23 ||
                    p > n
                )
                    continue;

                if(p <= root) {
                    calc(p);
                    continue;
                }

                /*
                 * 第一個 large prime。
                 */
                if(!large) {
                    q =
                        n / p;

                    q_right =
                        n / q;

                    mul =
                        q + 1;

                    limit =
                        ULLONG_MAX /
                        mul;

                    acc =
                        ans;

                    large =
                        true;
                }

                /*
                 * 同 quotient 區間不做 division。
                 */
                while(p > q_right) {
                    --q;

                    q_right =
                        n / q;

                    mul =
                        q + 1;

                    limit =
                        ULLONG_MAX /
                        mul;
                }

                /*
                 * 真正快 overflow 才 mod。
                 */
                if(acc > limit)
                    acc %= MOD;

                acc *= mul;
            }
        }
    }

    if(large)
        ans = acc % MOD;

    cout << ans << '\n';
}
