#include "testlib.h"
#include <vector>
using namespace std;
signed main(int argc, char* argv[]) {
	registerGen(argc, argv, 1);
	int n = atoi(argv[1]);
    cout << n << '\n';
    vector<int> A(2*n);
    for(int i = 0; i < n; i++) A[i] = A[i+n] = i;
    shuffle(A.begin(), A.end());
    for(int i = 0; i < n*2; i++) cout << A[i] << " \n"[i==n*2-1];
}
