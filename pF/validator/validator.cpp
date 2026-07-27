#include "testlib.h"
#include <vector>
using namespace std;
signed main(int argc, char* argv[]) {
	registerValidation(argc, argv);
    const int N = atoi(argv[1]);
	int n = inf.readInt(1, N, "n");
	inf.readEoln();
    vector<int> A = inf.readInts(2*n, 0, n-1, "A_i");
    inf.readEoln();
    vector<int> cnt(n);
    for(auto &d : A) cnt[d]++;
    for(int i = 0; i < n; i++) ensure(cnt[i] == 2);
	inf.readEof();
}
