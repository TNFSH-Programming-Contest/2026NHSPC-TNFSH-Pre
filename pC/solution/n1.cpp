#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, q;
    cin >> n >> m >> q;

    string s;
    cin >> s;

    vector<int> ans(2 * m + 3);

    auto top = [&](int c) { return c + 1; };
    auto bottom = [&](int c) { return m + 1 + m - c; };

    int L = 2 * m + 2;
    int R = m + 1;
    int cur = L;

    auto link = [&](int x, int y) {
        ans[x] = y;
        ans[y] = x;
    };

    for (int c = 0; c < m; ++c) {
        if (s[c] == '.') {
            link(top(c), bottom(c));
        } else if (s[c] == '/') {
            link(cur, top(c));
            cur = bottom(c);
        } else {
            link(cur, bottom(c));
            cur = top(c);
        }
    }

    link(cur, R);

    while (q--) {
        int x;
        cin >> x;
        cout << ans[x] << '\n';
    }
}