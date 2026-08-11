#include <bits/stdc++.h>
using namespace std;
#define ll long long
//#define int ll
#pragma GCC optimize("O3,unroll-loops")
#pragma omp parallel for
#define push_back emplace_back
const int dx[] = {1, 0, -1, 0};
const int dy[] = {0, -1, 0, 1};
signed main() {
    ios::sync_with_stdio(false), cin.tie(0);
    int n, m, q; cin >> n >> m >> q;
    vector<string> v(n);
    for(auto &d : v) cin >> d;
    vector<int> ans(n*2+m*2+1);
    auto S = [&](int id) -> array<int, 3> {
        if(id <= m) return {0, id, 0};
        if(id <= n+m) return {id-m, m+1, 1};
        if(id <= n+m+m) return {n+1, m-(id-n-m-1), 2};
        return {n-(id-n-m-m-1), 0, 3};
    };
    auto T = [&](int a, int b) -> int {
        if(a == 0) return b;
        if(b == m+1) return a+m;
        if(a == n+1) return m-b+n+m+1;
        return n-a+n+m+m+1;
    };
    for(int i = 1; i <= n*2+m*2; i++) if(!ans[i]) {
        auto [a, b, c] = S(i);
        while(true) {
            int aa = a + dx[c];
            int bb = b + dy[c];
            a = aa, b = bb;
            if(1 <= a && a <= n && 1 <= b && b <= m) {
                if(v[a-1][b-1] == '\\') c ^= 3;
                if(v[a-1][b-1] == '/') c ^= 1;
            } else break;
        }
        int j = T(a, b);
        ans[i] = j;
        ans[j] = i;
    }
    while(q--) {
        int a; cin >> a;
        cout << ans[a] << '\n';
    }
}
