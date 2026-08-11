#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, q;
    cin >> n >> m >> q;

    vector<string> a(n);
    for (auto &s : a) cin >> s;

    vector<int> ans(m + 1);

    int dr[] = {-1, 0, 1, 0};
    int dc[] = {0, 1, 0, -1};

    while (q--) {
        int x;
        cin >> x;

        if (ans[x]) {
            cout << ans[x] << '\n';
            continue;
        }

        int r = 0, c = x - 1, d = 2;

        while (0 <= r && r < n && 0 <= c && c < m) {
            if (a[r][c] == '/') d ^= 1;
            if (a[r][c] == '\\') d ^= 3;
            r += dr[d];
            c += dc[d];
        }

        int y;

        if (r < 0) y = c + 1;
        else if (c >= m) y = m + r + 1;
        else if (r >= n) y = m + n + m - c;
        else y = 2 * m + n + n - r;

        ans[x] = y;

        if (y <= m) ans[y] = x;

        cout << y << '\n';
    }
}