#include <bits/stdc++.h>
using namespace std;

using ll = long long;

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

    // ------------------------------------------------------------
    // Odd-only bitset
    //
    // bit i <=> number = 2*i + 1
    //
    // 1 = composite
    // 0 = prime
    // ------------------------------------------------------------

    const int m = (n >> 1) + 1;
    const int words = (m + 63) >> 6;

    vector<uint64_t> composite(words, 0);

    // 1 is not prime
    composite[0] = 1ULL;

    // Remove bits beyond n
    if (m & 63)
        composite.back() |= ~((1ULL << (m & 63)) - 1ULL);

    // ------------------------------------------------------------
    // Eratosthenes sieve
    // ------------------------------------------------------------

    const int limit = sqrt((double)n);

    for (int p = 3; p <= limit; p += 2) {
        const int pi = p >> 1;

        if (composite[pi >> 6] & (1ULL << (pi & 63)))
            continue;

        // index of p*p
        int idx = (p * p) >> 1;

        int w = idx >> 6;
        unsigned b = idx & 63;

        /*
            Every next composite:

                number += 2p
                index  += p

            Therefore in bitset:

                b += p
                w += carry

            We split p into:

                p = q * 64 + r

            so each iteration costs no division/modulo.
        */

        const unsigned q = p >> 6;
        const unsigned r = p & 63;

        while (w < words) {
            composite[w] |= 1ULL << b;

            // Move by p bits
            b += r;
            w += q;

            if (b >= 64) {
                b -= 64;
                ++w;
            }
        }
    }

    // ------------------------------------------------------------
    // Calculate answer
    // ------------------------------------------------------------

    int ans = 1;

    // Prime 2
    {
        int now = 0;

        for (ll pw = 2; pw <= n; pw *= 2)
            now += n / pw;

        ans = (ll)ans * (now + 1) % MOD;
    }

    // ------------------------------------------------------------
    // Enumerate odd primes by scanning 64 bits at once
    // ------------------------------------------------------------

    for (int w = 0; w < words; ++w) {
        uint64_t x = ~composite[w];

        while (x) {
            const int bit = __builtin_ctzll(x);
            const int idx = (w << 6) + bit;

            if (idx >= m)
                break;

            const int p = (idx << 1) + 1;

            int now = 0;

            for (ll pw = p; pw <= n; pw *= p)
                now += n / pw;

            ans = (ll)ans * (now + 1) % MOD;

            // remove lowest set bit
            x &= x - 1;
        }
    }

    cout << ans << '\n';
}