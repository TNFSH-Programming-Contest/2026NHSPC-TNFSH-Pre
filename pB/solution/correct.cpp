#include <bits/stdc++.h>

using loli = long long;

#define nitrogen std::ios::sync_with_stdio(false); std::cin.tie(nullptr);

int main() {
	nitrogen;

    int n;
    std::cin >> n;

    std::vector<loli> a(n);
    for (auto &x : a) std::cin >> x;

    std::sort(a.begin(), a.end());

    std::vector<loli> pre(n + 1);
    for (int i = 0; i < n; ++i) {
        pre[i + 1] = pre[i] + a[i];
    }

    constexpr loli M = 43200;
    loli ans = std::numeric_limits<loli>::max();

    for (int i = 0; i < n; ++i) {
        loli left = (i + 1LL) * a[i] - pre[i + 1];

        loli right = (n - i - 1LL) * (M + a[i]) - (pre[n] - pre[i + 1]);

        ans = std::min(ans, left + right);
    }

    std::cout << ans << '\n';
}