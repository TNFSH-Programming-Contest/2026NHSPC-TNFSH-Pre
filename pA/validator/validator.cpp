#include "testlib.h"

int main(int argc, char* argv[]) {
    registerValidation(argc, argv);

    ensuref(argc >= 2, "validator requires the maximum A_i as argv[1]");
    const long long maxValue = std::stoll(argv[1]);

    const int n = inf.readInt(1, 2000, "N");
    inf.readSpace();
    inf.readInt(1, n, "K");
    inf.readEoln();

    for (int i = 0; i < n; ++i) {
        inf.readLong(0LL, maxValue, "A_i");
        if (i + 1 == n) {
            inf.readEoln();
        } else {
            inf.readSpace();
        }
    }
    inf.readEof();
}
