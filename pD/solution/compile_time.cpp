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

constexpr int PREP[4] = {
    7, 11, 13, 17
};

constexpr int PRE_WHEEL =
    2 * 3 * 5 * 7 * 11 * 13 * 17;

constexpr int PRE_BLOCKS =
    PRE_WHEEL / 30; // 17017

// 跟 presieve period 分開
constexpr int SEG_BLOCKS = 1 << 15; // 32768

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

/* compile-time Eratosthenes */

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

        for(int i = 19; i <= LIM; ++i) {
            if((bit[i >> 6] >> (i & 63)) & 1ULL)
                prime[cnt++] = i;
        }
    }
};

constexpr PrimeTable PT;

/* compile-time presieve */

struct Presieve {
    array<uint8_t, PRE_BLOCKS> a{};

    constexpr Presieve() {
        for(int i = 0; i < PRE_BLOCKS; ++i)
            a[i] = 0xff;

        for(int z = 0; z < 4; ++z) {
            int p = PREP[z];

            for(int bit = 0; bit < 8; ++bit) {
                int b = 0;

                while((30 * b + REM[bit]) % p)
                    ++b;

                for(; b < PRE_BLOCKS; b += p)
                    a[b] &= uint8_t(~(1u << bit));
            }
        }
    }
};

constexpr Presieve PRE;

alignas(64) uint8_t seg[SEG_BLOCKS];

uint32_t nxt[4000][8];

/*
 * PRE 的 period 是 PRE_BLOCKS，
 * segment 可以任意大。
 */
inline void fill_presieve(uint32_t baseBlock, int blocks) {
    int off = baseBlock % PRE_BLOCKS;
    int done = 0;

    int take = min(blocks, PRE_BLOCKS - off);

    memcpy(seg, PRE.a.data() + off, take);

    done += take;

    while(done < blocks) {
        take = min(blocks - done, PRE_BLOCKS);
        memcpy(seg + done, PRE.a.data(), take);
        done += take;
    }
}

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

        ans = 1LL * ans * (e + 1) % MOD;
    };

    for(int p : {2, 3, 5, 7, 11, 13, 17}) {
        if(p <= n)
            calc(p);
    }

    int root = sqrt((double)n);

    while(1LL * (root + 1) * (root + 1) <= n)
        ++root;

    while(1LL * root * root > n)
        --root;

    /*
     * 初始化每個 p 的 8 條 strike stream。
     */
    for(int z = 0; z < PT.cnt; ++z) {
        int p = PT.prime[z];
        int pr = p % 30;

        for(int t = 0; t < 8; ++t) {
            int r = REM[t];

            int m = p;

            int d = r - m % 30;
            if(d < 0)
                d += 30;

            m += d;

            ll x = 1LL * p * m;

            int bit =
                rem_id(pr * r % 30);

            nxt[z][bit] = uint32_t(x / 30);
        }
    }

    /*
     * p > sqrt(n)
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
        ll base = 30LL * baseBlock;

        ll R = min<ll>( n, base + 30LL * SEG_BLOCKS - 1);

        int blocks = int((R - base) / 30) + 1;

        fill_presieve(baseBlock, blocks);

        if(baseBlock == 0)
            seg[0] &= ~1u;

        uint32_t endBlock = baseBlock + blocks;

        /*
         * segmented Eratosthenes
         */
        for(int z = 0; z < PT.cnt; ++z) {
            uint32_t p = PT.prime[z];

            if(1ULL * p * p > (ull)R)
                break;

            for(int bit = 0; bit < 8; ++bit) {
                uint32_t b = nxt[z][bit];

                const uint8_t mask = uint8_t(~(1u << bit));

                for( ; b < endBlock; b += p) {
                    seg[b - baseBlock] &= mask;
                }

                nxt[z][bit] = b;
            }
        }

        /*
         * enumerate primes
         */
        for(int b = 0; b < blocks; ++b) {
            unsigned v = seg[b];

            while(v) {
                int bit = __builtin_ctz(v);
                v &= v - 1;

                ll p = base + 30LL * b + REM[bit];

                if(p < 19 || p > n)
                    continue;

                if(p <= root) {
                    calc(p);
                    continue;
                }

                if(!large) {
                    q = n / p;
                    q_right = n / q;

                    mul = q + 1;
                    limit = ULLONG_MAX / mul;

                    acc = ans;
                    large = true;
                }

                /*
                 * quotient 改變時才做 division。
                 */
                while(p > q_right) {
                    --q;
                    q_right = n / q;
                    mul = q + 1;
                    limit = ULLONG_MAX / mul;
                }

                /*
                 * 不再固定每 2 個就 modulo。
                 *
                 * 能乘就一直乘，
                 * 真正要 overflow 才 reduce。
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
