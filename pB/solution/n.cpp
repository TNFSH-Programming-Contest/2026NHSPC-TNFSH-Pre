#include <bits/stdc++.h>

using loli = long long int;

#define M 43200
#define nitrogen std::ios::sync_with_stdio(false), std::cin.tie(nullptr)

int main() {
    nitrogen;

    int n;
    std::cin >> n;
    std::vector<loli> count(M, 0);
    loli current_cost = 0;
    for (int i = 0; i < n; i++) {
        int t;
        std::cin >> t;
        count[t]++;

        if (t != 0) {
            current_cost += M - t;
        }
    }

    loli ans = current_cost;
    for (int time = 1; time < M; time++) {
        current_cost += n;
        current_cost -= (loli)M * count[time];

        ans = std::min(current_cost, ans);
    }

    std::cout << ans << '\n';

    return 0;
}