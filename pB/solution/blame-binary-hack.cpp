#include <bits/stdc++.h>
using namespace std;
#define ll long long
#pragma GCC optimize("Ofast")
int read() {
    int x = 0;
    char c = getchar();
    while(c && !isdigit(c)) c = getchar();
    while(isdigit(c)) x = (x<<3) + (x<<1) + (c^48), c = getchar();
    return x;
}
mt19937 rng(time(0) + 87);
signed main() {
    ios::sync_with_stdio(false), cin.tie(0);
    int n = read();
    vector<int> cnt(43200);
    vector<int> TMP;
    for(int i = 0; i < n; i++) {
        int a = read();
        cnt[a]++;
        TMP.push_back(a);
    }
    sort(TMP.begin(), TMP.end());
    TMP.resize(unique(TMP.begin(), TMP.end()) - TMP.begin());
    //for(int i = 1; i < TMP.size(); i++) swap(TMP[i], TMP[i%rng()]);
    sort(TMP.begin(), TMP.end(), [&](const auto &A, const auto &B) {return cnt[A] > cnt[B];});
    ll ans = 43200LL*n;
    // Heuristic pruning: only inspect the most frequent 512 candidate times.
    // The optimum need not be frequent, so this is not an exact algorithm.
    int magic = 512;
    for(auto &i : TMP) if(cnt[i]) {
        ll tmp = 0;
        for(auto &j : TMP) {
            tmp += 1ll*cnt[j]*(j<= i ? i-j : i+43200-j);
            if(tmp > ans) break;
        }
        if((--magic <= 0)) {
            ans = min(ans, tmp);
            break;
        }
        ans = min(ans, tmp);
    }
    cout << ans << '\n';
}
