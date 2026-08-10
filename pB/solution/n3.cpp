#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const int MOD = 43200;

    int W;
    cin >> W;

    vector<int> t(W);

    for (int i = 0; i < W; i++) {
        cin >> t[i];
    }

    long long ans = LLONG_MAX;

    for (int i = 0; i < W; i++) {

        int target = t[i];

        long long sum = 0;

        for (int j = 0; j < W; j++) {

            int same = 0;
            bool first = true;

            for (int k = 0; k < W; k++) {

                if (t[k] == t[j]) {
                    same++;

                    if (k < j) {
                        first = false;
                    }
                }
            }

            if (!first)
                continue;

            int d = target - t[j];

            if (d < 0) {
                d += MOD;
            }

            sum += 1LL * same * d;
        }

        ans = min(ans, sum);
    }

    cout << ans << '\n';

    return 0;
}