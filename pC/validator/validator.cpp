#include "testlib.h"

#include <string>

int main(int argc, char* argv[]) {
    registerValidation(argc, argv);

    ensuref(argc >= 5, "validator requires maxN, maxM, maxQ, and dotsOnly");
    const int maxN = std::stoi(argv[1]);
    const int maxM = std::stoi(argv[2]);
    const int maxQ = std::stoi(argv[3]);
    const bool dotsOnly = std::string(argv[4]) == "1";

    ensuref(1 <= maxN && maxN <= 2000, "maxN must be in [1, 2000]");
    ensuref(1 <= maxM && maxM <= 2000, "maxM must be in [1, 2000]");
    ensuref(1 <= maxQ && maxQ <= 200000, "maxQ must be in [1, 200000]");

    const int n = inf.readInt(1, maxN, "N");
    inf.readSpace();
    const int m = inf.readInt(1, maxM, "M");
    inf.readSpace();
    const int q = inf.readInt(1, maxQ, "Q");
    inf.readEoln();

    for (int row = 0; row < n; ++row) {
        const std::string cells = inf.readToken();
        ensuref(static_cast<int>(cells.size()) == m,
                "grid row %d has length %d instead of %d", row + 1,
                static_cast<int>(cells.size()), m);
        for (char cell : cells) {
            ensuref(cell == '.' || cell == '/' || cell == '\\',
                    "invalid grid character");
            if (dotsOnly) ensuref(cell == '.', "straight subtask requires only dots");
        }
        inf.readEoln();
    }

    const int perimeter = 2 * (n + m);
    for (int query = 0; query < q; ++query) {
        inf.readInt(1, perimeter, "a");
        inf.readEoln();
    }
    inf.readEof();
}
