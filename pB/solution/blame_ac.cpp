#include <bits/stdc++.h>
using namespace std;
#define ll long long
signed main() {
    ios::sync_with_stdio(false), cin.tie(0);
    int n; cin >> n;
    int m = 43200;
    vector<int> cnt(m);
    ll sum = 0;
    for(int i = 1; i <= n; i++) {
        int a; cin >> a;
        cnt[a]++;
        sum += a;
    }
    int now = n;
    ll ans = LLONG_MAX;
    for(int i = 0; i < m; i++) {
        now -= cnt[i];
        ans = min(ans, 1ll*m*now + 1ll*n*i - sum);
    }
    cout << ans << '\n';
}
