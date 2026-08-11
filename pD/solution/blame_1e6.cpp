#include <bits/stdc++.h>
using namespace std;
#define ll long long
#define int ll
#pragma GCC optimize("Ofast")
const int mod = 1145141;
signed main() {
    ios::sync_with_stdio(false), cin.tie(0);
    int n; cin >> n;
    int ans = 1;
    vector<int> isprime(n+1, 1);
    for(int i = 2; i <= n; i++) if(isprime[i]) {
        int now = 0;
        for(int j = i+i; j <= n; j+=i) isprime[j] = 0;
        for(int j = i; j <= n; j *= i) {
            now += n/j;
        }
        ans = ans * (now+1)%mod;
    }
    cout << ans << '\n';
}
