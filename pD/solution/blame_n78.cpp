#include <bits/stdc++.h>
using namespace std;
#define ll long long
const int mod = 1145141;
int fpw(int a, int b) {
    if(b == 0) return 1;
    int re = fpw(a, b>>1);
    if(b&1) return 1ll*re*re%mod*a%mod;
    return 1ll*re*re%mod;
}
int pi(int n) {
    int sn = sqrt(n);
    vector<bool> isprime(sn+1, 1);
    vector<int> primes;
    for(int i = 2; i <= sn; i++) if(isprime[i]) {
        primes.push_back(i);
        for(ll j = 1ll*i*i; j <= sn; j += i) isprime[j] = 0;
    }
    vector<pair<int, int> > v;
    vector<int> mp(sn+5), mp2(sn+5);
    int N = 0;
    for(int l = 1, r; l <= n; l = r + 1) {
        r = n/(n/l);
        int now = n/l;
        v.push_back({now, now-1});
        if(now <= sn) mp[now] = N;
        else mp2[n/now] = N;
        N++;
    }
    for(int i = 0; i < primes.size(); i++) {
        auto &p = primes[i];
        for(int j = 0; j < N && 1ll*p * p <= v[j].first; j++) {
            int fr = v[j].first/p;
            int id = fr <= sn ? mp[fr] : mp2[n/fr];
            v[j].second -= (v[id].second - (i));
        }
    }
    return v[0].second;
}
signed main() {
    ios::sync_with_stdio(false), cin.tie(0);
    int n; cin >> n;
    int sn = sqrt(n);
    vector<bool> isprime(sn+1, 1);
    int ans = 1;
    for(int i = 2; i <= sn; i++) if(isprime[i]) {
        for(ll j = 1ll*i*i; j <= sn; j+=i) isprime[j] = 0;
        int now = 0;
        for(ll j = i; j <= n; j *= i)
            now += n/j;
        ans = 1ll*ans * (now + 1)%mod;
    }
    for(int l = sn+1, r; l <= n; l = r+1) {
        r = (n/(n/l));
        int k = n/l;
        ans = 1ll*ans * fpw(k+1, pi(r) - pi(l-1))%mod;
    }
    cout << ans << '\n';
}