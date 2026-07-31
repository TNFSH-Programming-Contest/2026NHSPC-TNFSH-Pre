#include "testlib.h"

#include <string>

int main(int argc, char* argv[]) {
    registerValidation(argc, argv);
    ensuref(argc >= 3, "validator requires maxN and maxQ");

    const int maxN = std::stoi(argv[1]);
    const int maxQ = std::stoi(argv[2]);
    ensuref(2 <= maxN && maxN <= 2000, "maxN must be in [2, 2000]");
    ensuref(1 <= maxQ && maxQ <= 100000,
            "maxQ must be in [1, 100000]");

    const int n = inf.readInt(2, maxN, "n");
    inf.readSpace();
    const int q = inf.readInt(1, maxQ, "Q");
    inf.readSpace();
    inf.readLong(1, 1000000000LL, "scale");
    inf.readEoln();

    for (int r = 1; r < n; ++r) {
        for (int l = 0; l < r; ++l) {
            inf.readInt(0, 15, "delta");
            if (l + 1 == r) {
                inf.readEoln();
            } else {
                inf.readSpace();
            }
        }
    }

    for (int i = 0; i < q; ++i) {
        const int l = inf.readInt(0, n - 2, "l");
        inf.readSpace();
        inf.readInt(l + 1, n - 1, "r");
        inf.readEoln();
    }
    inf.readEof();
}
