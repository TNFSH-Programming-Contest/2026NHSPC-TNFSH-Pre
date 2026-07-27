#include <iostream>
#include <vector>
using namespace std;
int main() {
    ios::sync_with_stdio(false), cin.tie(0);
    int n; cin >> n;
    vector<int> A(2*n);
    for(auto &d : A) cin >> d;
    long long ans = 0;
    for(int i = 0; i < n*2; i++) {
        for(int t = 0; t < 1+(i != n*2-1 && A[i] == A[i+1]); t++) {
            int l = i, r = i+t;
            int mx = A[l];
            while(true) {
                ans += mx;
                if(l == 0 || r == n*2-1) break;
                if(A[l-1] != A[r+1]) break;
                l--, r++;
                mx = max(mx, A[l]);
            }
        }
    }
    cout << ans << '\n';
}
