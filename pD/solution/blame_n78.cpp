#include <bits/stdc++.h>
using namespace std;
#define ll long long
#define int ll
const int mod = 1145141;
int fpw(int a, int b) {
    if(b == 0) return 1;
    int re = fpw(a, b>>1);
    if(b&1) return re*re%mod*a%mod;
    return re*re%mod;
}
int pi(int n) {
    int sn = sqrt(n);
    vector<int> isprime(sn+1, 1), primes;
    for(int i = 2; i <= sn; i++) if(isprime[i]) {
        primes.push_back(i);
        for(int j = i*i; j <= sn; j += i) isprime[j] = 0;
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
        for(int j = 0; j < N && p * p <= v[j].first; j++) {
            int fr = v[j].first/p;
            int id = fr <= sn ? mp[fr] : mp2[n/fr];
            v[j].second -= (v[id].second - (i));
        }
    }
    return N == 0 ? n-1 : v[0].second;
}
signed main() {
    // (n^(7/8))/log2(n)
    ios::sync_with_stdio(false), cin.tie(0);
    int n; cin >> n;
    int sn = sqrt(n);
    //int sn = n;
    vector<int> isprime(sn+1, 1);
    int ans = 1;
    for(int i = 2; i <= sn; i++) if(isprime[i]) {
        for(int j = i+i; j <= sn; j+=i) isprime[j] = 0;
        int now = 0;
        for(int j = i; j <= n; j *= i)
            now += n/j;
        ans = ans * (now + 1)%mod;
    }
    //cout << ans  << '\n';

    for(int l = sn+1, r; l <= n; l = r+1) {
        r = (n/(n/l));
        int k = n/l;
        //cout << "! " << l << ' ' << r << ' ' << k << '\n';

        ans = ans * fpw(k+1, pi(r) - pi(l-1))%mod;
    }
    cout << ans << '\n';
}