#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, q;
    cin >> n >> m >> q;

    vector<string> a(n);
    char type = '/';

    for (auto &s : a) {
        cin >> s;
        for (char c : s)
            if (c != '.') type = c;
    }

    vector<int> ans(2 * (n + m) + 1);

    auto top = [&](int c) { return c + 1; };
    auto right = [&](int r) { return m + r + 1; };
    auto bottom = [&](int c) { return m + n + m - c; };
    auto left = [&](int r) { return 2 * m + n + n - r; };

    if (type == '/') {
        vector<int> v(m);
        for (int c = 0; c < m; ++c) v[c] = bottom(c);

        for (int r = n - 1; r >= 0; --r) {
            int h = left(r);

            for (int c = 0; c < m; ++c)
                if (a[r][c] == '/') swap(h, v[c]);

            ans[right(r)] = h;
        }

        for (int c = 0; c < m; ++c)
            ans[top(c)] = v[c];

        for (int c = 0; c < m; ++c) v[c] = top(c);

        for (int r = 0; r < n; ++r) {
            int h = right(r);

            for (int c = m - 1; c >= 0; --c)
                if (a[r][c] == '/') swap(h, v[c]);

            ans[left(r)] = h;
        }

        for (int c = 0; c < m; ++c)
            ans[bottom(c)] = v[c];
    }

    else {
        vector<int> v(m);
        for (int c = 0; c < m; ++c) v[c] = bottom(c);

        for (int r = n - 1; r >= 0; --r) {
            int h = right(r);

            for (int c = m - 1; c >= 0; --c)
                if (a[r][c] == '\\') swap(h, v[c]);

            ans[left(r)] = h;
        }

        for (int c = 0; c < m; ++c)
            ans[top(c)] = v[c];

        for (int c = 0; c < m; ++c) v[c] = top(c);

        for (int r = 0; r < n; ++r) {
            int h = left(r);

            for (int c = 0; c < m; ++c)
                if (a[r][c] == '\\') swap(h, v[c]);

            ans[right(r)] = h;
        }

        for (int c = 0; c < m; ++c)
            ans[bottom(c)] = v[c];
    }

    while (q--) {
        int x;
        cin >> x;
        cout << ans[x] << '\n';
    }
}