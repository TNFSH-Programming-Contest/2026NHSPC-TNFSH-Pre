#include <iostream>
#include <vector>
#include <cassert>
using namespace std;
int main() {
    ios::sync_with_stdio(false), cin.tie(0);
    int n; cin >> n;
    assert(n <= 100);
    vector<int> A(n*2);
    for(auto &d : A) cin >> d;
    int ans = 0;
    for(int i = 0; i < n*2; i++) for(int j = i; j < n*2; j++) {
        int ok = 1;
        int mx = 0;
        for(int k = 0; ok && k < (j-i+2)/2; k++) {
            ok &= A[i+k] == A[j-k];
            mx = max(mx, A[i+k]);
        }
        if(ok) ans += mx;
    }
    cout << ans << '\n';
}
