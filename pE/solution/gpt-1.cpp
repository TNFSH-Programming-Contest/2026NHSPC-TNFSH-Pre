#include <bits/stdc++.h>
using namespace std;

namespace {

static const int MAXN = 2000;
static const int B = 4000000;

static int n;
static vector<int> opt;
static vector<long long> dp;

inline int &OPT(int l, int r) {
    return opt[l * n + r];
}

inline long long &DP(int l, int r) {
    return dp[l * n + r];
}

inline void putbit(string &s, int x) {
    s.push_back(char('0' + x));
}

} // namespace

string encode(
    int N,
    const array<array<long long, 2000>, 2000> &w
) {
    n = N;

    opt.assign(n * n, 0);
    dp.assign(n * n, 0);

    for (int i = 0; i < n; ++i)
        OPT(i, i) = i;

    /*
        Knuth optimization.

        opt[l][r-1] <= opt[l][r] <= opt[l+1][r]

        We choose the leftmost optimum.
    */

    const long long INF = (1LL << 62);

    for (int len = 2; len <= n; ++len) {
        for (int l = 0; l + len <= n; ++l) {
            int r = l + len - 1;

            int L = OPT(l, r - 1);
            int R = OPT(l + 1, r);

            L = max(L, l);
            R = min(R, r - 1);

            long long best = INF;
            int bestk = L;

            for (int k = L; k <= R; ++k) {
                long long cur =
                    DP(l, k) +
                    DP(k + 1, r) +
                    w[l][r];

                if (cur < best) {
                    best = cur;
                    bestk = k;
                }
            }

            DP(l, r) = best;
            OPT(l, r) = bestk;
        }
    }

    /*
        Encoding.

        For each fixed l:

            opt[l][l]
            opt[l][l+1]
            ...
            opt[l][n-1]

        is nondecreasing.

        Let

            delta = opt[l][r] - opt[l][r-1] >= 0.

        Encode delta as:

            111...110
            ^ delta ones

        Hence delta = 0 costs 1 bit.

        Number of entries:
            M = n(n-1)/2

        Sum of all deltas:
            <= M

        Total:
            <= 2M <= 3,998,000

        So this is ALWAYS below B=4,000,000.
    */

    const int M = n * (n - 1) / 2;

    string S;
    S.reserve(2 * M + 16);

    // Encode every row.
    for (int l = 0; l < n; ++l) {
        int last = l;

        for (int r = l + 1; r < n; ++r) {
            int cur = OPT(l, r);
            int delta = cur - last;

            // delta ones
            for (int t = 0; t < delta; ++t)
                S.push_back('1');

            // terminating zero
            S.push_back('0');

            last = cur;
        }
    }

    // Guaranteed:
    // S.size() <= 2*M <= 3,998,000 < 4,000,000.
    return S;
}

#include <bits/stdc++.h>
using namespace std;

namespace {

static int nDecode;
static vector<int> optDecode;

inline int &OPTDecode(int l, int r) {
    return optDecode[l * nDecode + r];
}


} // namespace

void decode(int N, const string &S) {
    nDecode = N;

    optDecode.assign(nDecode * nDecode, 0);

    for (int i = 0; i < nDecode; ++i)
        OPTDecode(i, i) = i;

    /*
        Decode unary delta.

        For every (l,r):

            1 ... 1 0
            \___/
            delta ones

        Therefore:
            delta = number of consecutive 1s
    */

    size_t p = 0;

    for (int l = 0; l < nDecode; ++l) {
        int last = l;

        for (int r = l + 1; r < nDecode; ++r) {
            int delta = 0;

            while (p < S.size() && S[p] == '1') {
                ++delta;
                ++p;
            }

            // The terminating zero.
            if (p < S.size())
                ++p;

            last += delta;
            OPTDecode(l, r) = last;
        }
    }
}

int query(int l, int r) {
    return OPTDecode(l, r);
}