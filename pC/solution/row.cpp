#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, q;
    cin >> n >> m >> q;

    vector<int> pos(n, -1), prv(n, -1), nxt(n, -1);
    vector<char> type(n);

    vector<int> first(m, -1), last(m, -1);

    for (int r = 0; r < n; ++r) {
        string s;
        cin >> s;

        for (int c = 0; c < m; ++c) {
            if (s[c] == '.') continue;

            pos[r] = c;
            type[r] = s[c];

            if (first[c] == -1) first[c] = r;

            if (last[c] != -1) {
                prv[r] = last[c];
                nxt[last[c]] = r;
            }

            last[c] = r;
            break;
        }
    }

    auto top = [&](int c) { return c + 1; };
    auto right = [&](int r) { return m + r + 1; };
    auto bottom = [&](int c) { return m + n + m - c; };
    auto left = [&](int r) { return 2 * m + n + n - r; };

    auto hitUp = [&](int r, int c) {
        if (r == -1) return top(c);
        return type[r] == '/' ? right(r) : left(r);
    };

    auto hitDown = [&](int r, int c) {
        if (r == -1) return bottom(c);
        return type[r] == '/' ? left(r) : right(r);
    };

    while (q--) {
        int x;
        cin >> x;

        if (x <= m) {
            int c = x - 1;
            int r = first[c];

            if (r == -1) cout << bottom(c);
            else cout << (type[r] == '/' ? left(r) : right(r));
        }

        else if (x <= m + n) {
            int r = x - m - 1;

            if (pos[r] == -1) {
                cout << left(r);
            } else {
                int c = pos[r];

                if (type[r] == '/')
                    cout << hitDown(nxt[r], c);
                else
                    cout << hitUp(prv[r], c);
            }
        }

        else if (x <= 2 * m + n) {
            int c = 2 * m + n - x;
            int r = last[c];

            if (r == -1) cout << top(c);
            else cout << (type[r] == '/' ? right(r) : left(r));
        }

        else {
            int r = 2 * m + 2 * n - x;

            if (pos[r] == -1) {
                cout << right(r);
            } else {
                int c = pos[r];

                if (type[r] == '/')
                    cout << hitUp(prv[r], c);
                else
                    cout << hitDown(nxt[r], c);
            }
        }

        cout << '\n';
    }
}