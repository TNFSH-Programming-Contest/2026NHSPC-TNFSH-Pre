#include "testlib.h"
using namespace std;

const int MAXN = 1000;

int main(int argc, char* argv[]) {
	registerValidation(argc, argv);
	// about testlib, see https://codeforces.com/blog/entry/18426

    const int MAXN = std::stoi(argv[1]);
    const bool T_SAME = argv[2][0] == '1';

	int n = inf.readInt(1, MAXN, "n");
	inf.readEoln();
    std::vector<int> times(n);
    for (int i = 0; i < n; i++) {
        int t = inf.readInt(0, 43200 - 1, "t");
        times[i] = t;
        if (i < n - 1) {
            inf.readSpace();
        } else {
            inf.readEoln();
        }
    }
    if (T_SAME) {
        for (int i = 1; i < n; i++) {
            ensuref(times[i] == times[0], "All times must be the same when T_SAME is true");
        }
    }
	inf.readEof();

	return 0;
}
