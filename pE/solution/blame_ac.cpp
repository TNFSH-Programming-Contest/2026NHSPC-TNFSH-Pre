#include <bits/stdc++.h>
using namespace std;
namespace {
    vector<vector<int> > opt;
}

void decode(int n, string s) {
    opt = vector<vector<int> > (n, vector<int> (n));
    int id = 0;
    for(int i = 0; i < n; i++) {
        int now = i;
        for(int j = i+1; j < n; j++) {
            id++;
            while(id < s.size() && s[id] != '0') id++, now++;
            opt[i][j] = now;
        }
    }
}

int query(int l, int r) {
    return opt[l][r];
}

#include <bits/stdc++.h>
using namespace std;

#pragma GCC optimize("Ofast")

string encode(int n, vector<vector<long long> > w) {
    const long long inf = 1e18;
    vector<vector<long long> > dp(n, vector<long long> (n, inf));
    vector<vector<int> > opt(n, vector<int> (n));
    for(int i = 0; i < n; i++) dp[i][i] = 0, opt[i][i] = i;
    for(int len = 2; len <= n; len++) {
        for(int i = 0, j = len-1; j < n; i++, j++) {
            for(int k = max(i, opt[i][j-1]); k < min(j, opt[i+1][j]+1); k++) {
                dp[i][j] = min(dp[i][j], dp[i][k] + dp[k+1][j] + w[i][j]);
                if(dp[i][k] + dp[k+1][j] + w[i][j] == dp[i][j]) opt[i][j] = k;
            }
        }
    }
    string re;
    for(int i = 0; i < n; i++) {
        int ls = i;
        for(int j = i+1; j < n; j++) {
            re += '0';
            re += string(opt[i][j] - ls, '1');
            ls = opt[i][j];
        }
    }
    return re;
}
