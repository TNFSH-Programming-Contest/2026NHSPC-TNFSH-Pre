#include "testlib.h"

#include <string>

enum class InputMode {
    ANY = 0,
    DOT_ONLY = 1,
    ALL_SAME_REFLECT = 2,
    AT_MOST_ONE_REFLECT_PER_ROW = 3,
};

int main(int argc, char* argv[]) {
    registerValidation(argc, argv);

    ensuref(argc == 6,
            "validator requires maxN, maxM, maxQ, inputMode, and "
            "allQueryFromUpper");
    const int maxN = std::stoi(argv[1]);
    const int maxM = std::stoi(argv[2]);
    const int maxQ = std::stoi(argv[3]);
    const int rawInputMode = std::stoi(argv[4]);
    const InputMode inputMode = static_cast<InputMode>(rawInputMode);
    const int rawAllQueryFromUpper = std::stoi(argv[5]);
    const bool allQueryFromUpper = rawAllQueryFromUpper;

    ensuref(1 <= maxN && maxN <= 2000, "maxN must be in [1, 2000]");
    ensuref(1 <= maxM && maxM <= 2000, "maxM must be in [1, 2000]");
    ensuref(1 <= maxQ && maxQ <= 200000, "maxQ must be in [1, 200000]");
    ensuref(0 <= rawInputMode && rawInputMode <= 3,
            "inputMode must be in [0, 3]");
    ensuref(0 <= rawAllQueryFromUpper && rawAllQueryFromUpper <= 1,
            "allQueryFromUpper must be 0 or 1");

    const int n = inf.readInt(1, maxN, "N");
    inf.readSpace();
    const int m = inf.readInt(1, maxM, "M");
    inf.readSpace();
    const int q = inf.readInt(1, maxQ, "Q");
    inf.readEoln();

    bool hasSlash = false;
    bool hasBackslash = false;
    for (int row = 0; row < n; ++row) {
        const std::string cells = inf.readToken();
        ensuref(static_cast<int>(cells.size()) == m,
                "grid row %d has length %d instead of %d", row + 1,
                static_cast<int>(cells.size()), m);
        int reflectCount = 0;
        for (char cell : cells) {
            ensuref(cell == '.' || cell == '/' || cell == '\\',
                    "invalid grid character");
            if (inputMode == InputMode::DOT_ONLY) {
                ensuref(cell == '.', "straight subtask requires only dots");
            }
            hasSlash |= cell == '/';
            hasBackslash |= cell == '\\';
            reflectCount += cell == '/' || cell == '\\';
        }
        if (inputMode == InputMode::AT_MOST_ONE_REFLECT_PER_ROW) {
            ensuref(reflectCount <= 1,
                    "row subtask requires at most one reflector per row");
        }
        inf.readEoln();
    }
    if (inputMode == InputMode::ALL_SAME_REFLECT) {
        ensuref(!hasSlash || !hasBackslash,
                "all_same_type requires all reflectors to have one type");
    }

    const int perimeter = 2 * (n + m);
    for (int query = 0; query < q; ++query) {
        int p = inf.readInt(1, perimeter, "a");
        if (allQueryFromUpper) {
            ensuref(p <= m, "all queries must be from the upper side");
        }
        inf.readEoln();
    }
    inf.readEof();
}
