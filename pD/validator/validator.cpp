#include "testlib.h"

int main(int argc, char* argv[]) {
    registerValidation(argc, argv);

    ensuref(argc >= 2, "validator requires the maximum n as argv[1]");
    const int maxN = std::stoi(argv[1]);
    ensuref(1 <= maxN && maxN <= 1000000000, "maximum n must be in [1, 1000000000]");

    inf.readInt(1, maxN, "n");
    inf.readEoln();
    inf.readEof();
}
