#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, q;
    cin >> n >> m >> q;

    vector<string> a(n);
    for (auto &s : a) cin >> s;

    int dr[] = {-1, 0, 1, 0};
    int dc[] = {0, 1, 0, -1};

    while (q--) {
        int x;
        cin >> x;

        int r, c, d;

        if (x <= m)
            r = 0, c = x - 1, d = 2;
        else if (x <= m + n)
            r = x - m - 1, c = m - 1, d = 3;
        else if (x <= 2 * m + n)
            r = n - 1, c = 2 * m + n - x, d = 0;
        else
            r = 2 * m + 2 * n - x, c = 0, d = 1;

        while (0 <= r && r < n && 0 <= c && c < m) {
            if (a[r][c] == '/') d ^= 1;
            if (a[r][c] == '\\') d ^= 3;
            r += dr[d];
            c += dc[d];
        }

        if (r < 0) cout << c + 1;
        else if (c >= m) cout << m + r + 1;
        else if (r >= n) cout << m + n + m - c;
        else cout << 2 * m + n + n - r;

        cout << '\n';
    }
}