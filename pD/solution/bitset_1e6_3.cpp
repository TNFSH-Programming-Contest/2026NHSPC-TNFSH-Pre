#include <bits/stdc++.h>
using namespace std;

using u64 = uint64_t;
using u32 = uint32_t;
using i64 = int64_t;

#pragma GCC optimize("Ofast,unroll-loops")
#pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")

static constexpr int MOD = 1145141;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    if (n < 2) {
        cout << 1 << '\n';
        return 0;
    }

    // ============================================================
    // Odd-only bitset
    //
    // bit i <=> number = 2*i + 1
    //
    // 0 = prime
    // 1 = composite
    //
    // For n = 1e9:
    //
    //   (n / 2) bits ~= 62.5 MB
    // ============================================================

    const int M = (n >> 1) + 1;
    const int WORDS = (M + 63) >> 6;

    vector<u64> composite(WORDS, 0);

    // 1 is not prime.
    composite[0] = 1ULL;

    // Padding bits are marked composite.
    if (M & 63) {
        composite[WORDS - 1] |=
            ~((1ULL << (M & 63)) - 1ULL);
    }

    // ============================================================
    // Integer sqrt
    // ============================================================

    int lim = (int)sqrt((double)n);

    while (1LL * (lim + 1) * (lim + 1) <= n)
        ++lim;

    while (1LL * lim * lim > n)
        --lim;

    // ============================================================
    // Eratosthenes sieve
    //
    // Only odd numbers are represented.
    //
    // p*p + 2p + 4p + ...
    //
    // In bitset index:
    //
    // idx += p
    // ============================================================

    for (int p = 3; p <= lim; p += 2) {
        const int pi = p >> 1;

        if ((composite[pi >> 6] >> (pi & 63)) & 1ULL)
            continue;

        int idx = (p * p) >> 1;

        int w = idx >> 6;
        unsigned b = idx & 63;

        const unsigned q = p >> 6;
        const unsigned r = p & 63;

        /*
            p < 64:

            q == 0

            This is the hottest small-prime case.
        */
        if (p < 64) {
            while (w < WORDS) {
                composite[w] |= 1ULL << b;

                b += r;

                if (b >= 64) {
                    b -= 64;
                    ++w;
                }
            }
        }

        /*
            p >= 64:

            Every iteration advances at least one word.
        */
        else {
            while (w < WORDS) {
                composite[w] |= 1ULL << b;

                b += r;
                w += q;

                if (b >= 64) {
                    b -= 64;
                    ++w;
                }
            }
        }
    }

    // ============================================================
    // Answer
    // ============================================================

    int ans = 1;

    // ------------------------------------------------------------
    // p = 2
    // ------------------------------------------------------------

    {
        int now = 0;

        for (i64 pw = 2; pw <= n; pw <<= 1)
            now += n / pw;

        ans = (i64)ans * (now + 1) % MOD;
    }

    // ------------------------------------------------------------
    // Prime enumeration
    // ------------------------------------------------------------

    for (int w = 0; w < WORDS; ++w) {

        /*
            Prime = zero bit in composite.
        */
        u64 x = ~composite[w];

        while (x) {

            const unsigned bit = __builtin_ctzll(x);
            const int idx = (w << 6) + bit;

            if (idx >= M)
                break;

            const int p = (idx << 1) + 1;

            int now;

            // ====================================================
            // p > sqrt(n)
            //
            // p^2 > n
            //
            // Therefore:
            //
            // v_p(n!) = floor(n/p)
            //
            // No power loop needed.
            // ====================================================

            if (p > lim) {

                /*
                    Floating-point division is used only as a fast
                    quotient estimator.

                    n <= 1e9, so double has vastly more than enough
                    precision for this quotient.

                    The correction makes the result exact.
                */

                u32 q = (u32)((double)n / (double)p);

                // Correct possible round-down.
                while ((u64)(q + 1) * (u32)p <= (u32)n)
                    ++q;

                // Correct possible round-up.
                while ((u64)q * (u32)p > (u32)n)
                    --q;

                now = (int)q;
            }

            // ====================================================
            // p <= sqrt(n)
            //
            // Need:
            //
            // floor(n/p)
            // + floor(n/p^2)
            // + floor(n/p^3)
            // + ...
            // ====================================================

            else {

                now = 0;

                i64 pw = p;

                while (pw <= n) {
                    now += n / pw;

                    /*
                        Avoid:
                            pw *= p
                        overflow concerns and unnecessary branch.
                    */
                    if (pw > n / p)
                        break;

                    pw *= p;
                }
            }

            ans = (i64)ans * (now + 1) % MOD;

            // Remove lowest set bit.
            x &= x - 1;
        }
    }

    cout << ans << '\n';
    return 0;
}