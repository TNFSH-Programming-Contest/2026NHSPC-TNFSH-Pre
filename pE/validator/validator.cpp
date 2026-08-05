#include "testlib.h"

#include <string>
#include <vector>

namespace {

constexpr long long MAX_DELTA = 30000000000000000LL;
constexpr long long MAX_COST = 30000000000000000LL;

}  // namespace

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
    const long long scale = inf.readLong(1, 1000000000LL, "scale");
    inf.readEoln();

    std::vector<long long> delta(n);
    std::vector<long long> lastCost(n);
    for (int r = 1; r < n; ++r) {
        for (int l = 0; l < r; ++l) {
            const long long minimumDelta = l + 1 == r ? -1 : 0;
            delta[l] = inf.readLong(minimumDelta, MAX_DELTA, "delta");
            if (l + 1 == r) {
                inf.readEoln();
            } else {
                inf.readSpace();
            }
        }
        __int128 extensionCost = 1;
        for (int l = r - 1; l >= 0; --l) {
            extensionCost += delta[l];
            const __int128 current =
                lastCost[l] + extensionCost * scale;
            ensuref(current <= MAX_COST,
                    "expanded w[%d][%d] exceeds 3e16", l, r);
            lastCost[l] = static_cast<long long>(current);
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
