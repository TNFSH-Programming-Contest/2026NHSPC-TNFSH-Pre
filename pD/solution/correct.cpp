#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

static const int64 MOD = 1145141;

// ------------------------------------------------------------
// integer sqrt
// ------------------------------------------------------------
int64 isqrt(int64 n) {
    int64 r = sqrtl((long double)n);

    while ((r + 1) * (r + 1) <= n) ++r;
    while (r * r > n) --r;

    return r;
}

// ------------------------------------------------------------
// fast pow
// ------------------------------------------------------------
int64 mod_pow(int64 a, int64 e) {
    int64 r = 1;

    while (e) {
        if (e & 1)
            r = r * a % MOD;

        a = a * a % MOD;
        e >>= 1;
    }

    return r;
}

// ------------------------------------------------------------
// Lucy DP
//
// 回傳所有 floor(N / i) 對應的 pi(x)
//
// values:
//   N, N/2, ..., N/r,
//   r-1, r-2, ..., 1
//
// 其中 r = floor(sqrt(N))
//
// g[x] 初始為 x-1，之後篩掉 composite。
// 最後 g[x] = pi(x)。
// ------------------------------------------------------------
struct LucyPi {
    int64 N;
    int r;

    vector<int64> values;
    vector<int64> g;

    // idSmall[x]:
    // x <= r 時，x 在 values 裡的位置
    vector<int> idSmall;

    LucyPi(int64 n) : N(n) {
        r = (int)isqrt(N);

        values.reserve(2 * r + 5);

        // N/1, N/2, ..., N/r
        for (int i = 1; i <= r; ++i)
            values.push_back(N / i);

        // 小數字部分
        int64 last = values.back();

        for (int64 x = last - 1; x >= 1; --x)
            values.push_back(x);

        g.resize(values.size());

        for (int i = 0; i < (int)values.size(); ++i)
            g[i] = values[i] - 1;

        idSmall.assign(r + 1, -1);

        // 找到 1...r 在 values 中的位置
        //
        // 如果 N/r == r，r 已經在第一段；
        // 否則 r 在第二段。
        int64 lastValue = values[r - 1];

        for (int x = 1; x <= r; ++x) {
            if (x == lastValue) {
                idSmall[x] = r - 1;
            } else if (x < lastValue) {
                // 第二段是:
                // lastValue-1, lastValue-2, ..., 1
                idSmall[x] =
                    r + (int)(lastValue - 1 - x);
            }
        }

        // Lucy sieve
        for (int p = 2; 1LL * p * p <= N; ++p) {
            int64 gp = get(p);

            // 如果 pi(p) > pi(p-1)，p 是質數
            if (gp == get(p - 1))
                continue;

            int64 sp = get(p - 1);

            // values 是遞減的
            for (int i = 0; i < (int)values.size(); ++i) {
                int64 v = values[i];

                if (v < 1LL * p * p)
                    break;

                g[i] -= get(v / p) - sp;
            }
        }
    }

    // values 中的 x 對應位置
    int getId(int64 x) const {
        if (x <= r) {
            return idSmall[x];
        }

        // x 必為 N / k 的形式
        return (int)(N / x) - 1;
    }

    int64 get(int64 x) const {
        return g[getId(x)];
    }
};

// ------------------------------------------------------------
// sieve primes <= sqrt(N)
//
// 用來處理 p <= sqrt(N) 的真正 exponent。
// ------------------------------------------------------------
vector<int> getPrimes(int n) {
    vector<bool> composite(n + 1, false);
    vector<int> primes;

    for (int i = 2; i <= n; ++i) {
        if (!composite[i]) {
            primes.push_back(i);

            if (1LL * i * i <= n) {
                for (int j = i * i; j <= n; j += i)
                    composite[j] = true;
            }
        }
    }

    return primes;
}

// ------------------------------------------------------------
// v_p(N!)
//
// Legendre:
// N/p + N/p^2 + ...
// ------------------------------------------------------------
int64 vp_factorial(int64 N, int64 p) {
    int64 res = 0;

    while (N) {
        N /= p;
        res += N;
    }

    return res;
}

// ------------------------------------------------------------
// solve
// ------------------------------------------------------------
int64 solve(int64 N) {
    if (N <= 1)
        return 1;

    int64 R = isqrt(N);

    // --------------------------------------------------------
    // Lucy DP:
    // pi(floor(N / k)) for all relevant k
    // --------------------------------------------------------
    LucyPi lp(N);

    int64 ans = 1;

    // --------------------------------------------------------
    // 1. p <= sqrt(N)
    //
    // 對這些質數必須完整計算：
    //
    // e_p = floor(N/p) + floor(N/p^2) + ...
    // --------------------------------------------------------
    vector<int> primes = getPrimes((int)R);

    for (int p : primes) {
        int64 e = vp_factorial(N, p);

        ans = ans * ((e + 1) % MOD) % MOD;
    }

    // --------------------------------------------------------
    // 2. p > sqrt(N)
    //
    // 因為 p^2 > N：
    //
    // e_p = floor(N/p)
    //
    // 如果 floor(N/p) = k，
    //
    // N/(k+1) < p <= N/k
    //
    // 所以質數數量：
    //
    // pi(N/k) - pi(N/(k+1))
    // --------------------------------------------------------

    int64 K = N / (R + 1);

    for (int64 k = 1; k <= K; ++k) {
        int64 cnt =
            lp.get(N / k)
            - lp.get(N / (k + 1));

        if (cnt == 0)
            continue;

        ans = ans * mod_pow(k + 1, cnt) % MOD;
    }

    return ans;
}

// ------------------------------------------------------------
// main
// ------------------------------------------------------------
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int64 N;
    cin >> N;

    cout << solve(N) << '\n';

    return 0;
}