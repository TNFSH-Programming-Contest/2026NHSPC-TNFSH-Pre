#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, q;
    cin >> n >> m >> q;

    vector<string> a(n);
    for (auto &s : a) cin >> s;

    int dr[4] = {-1, 0, 1, 0};
    int dc[4] = {0, 1, 0, -1};

    while (q--) {
        int x;
        cin >> x;

        int r, c, d;

        if (x <= m) {
            r = 0, c = x - 1, d = 2;
        } else if (x <= m + 2) {
            r = x - m - 1, c = m - 1, d = 3;
        } else if (x <= 2 * m + 2) {
            r = 1, c = 2 * m + 2 - x, d = 0;
        } else {
            r = 2 * m + 4 - x, c = 0, d = 1;
        }

        while (0 <= r && r < 2 && 0 <= c && c < m) {
            if (a[r][c] == '/') d ^= 1;
            else if (a[r][c] == '\\') d ^= 3;

            r += dr[d];
            c += dc[d];
        }

        if (r < 0)
            cout << c + 1;
        else if (c >= m)
            cout << m + r + 1;
        else if (r >= 2)
            cout << m + 2 + m - c;
        else
            cout << 2 * m + 2 + 2 - r;

        cout << '\n';
    }
}