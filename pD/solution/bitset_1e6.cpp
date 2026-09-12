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

    /*
        bitset:
        1 = candidate prime
        0 = composite

        只存奇數：
        bit k <=> number (2*k+1)
    */
    const int m = (n >> 1) + 1;
    const int W = (m + 63) >> 6;

    vector<uint64_t> bits(W, ~0ULL);

    // 1 非質數
    bits[0] &= ~1ULL;

    // 清掉超過 n 的垃圾 bit
    if (m & 63)
        bits.back() &= (1ULL << (m & 63)) - 1;

    // sieve
    for (int p = 3; 1LL * p * p <= n; p += 2) {
        if (!(bits[p >> 7] & (1ULL << ((p >> 1) & 63))))
            continue;

        const int idx = p >> 1;
        const int step = p;

        // number = p*p
        // 對應 index = p*p / 2
        int j = (p * p) >> 1;

        // 每次 number += 2p
        // index += p
        for (; j < m; j += step)
            bits[j >> 6] &= ~(1ULL << (j & 63));
    }

    int ans = 1;

    // p = 2
    {
        int now = 0;

        for (ll pw = 2; pw <= n; pw *= 2)
            now += n / pw;

        ans = (ll)ans * (now + 1) % MOD;
    }

    // odd primes
    for (int p = 3; p <= n; p += 2) {
        if (!(bits[p >> 7] & (1ULL << ((p >> 1) & 63))))
            continue;

        int now = 0;

        for (ll pw = p; pw <= n; pw *= p)
            now += n / pw;

        ans = (ll)ans * (now + 1) % MOD;
    }

    cout << ans << '\n';
}