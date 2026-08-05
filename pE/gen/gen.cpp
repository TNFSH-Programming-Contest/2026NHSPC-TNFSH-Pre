#include "testlib.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr int MAX_DELTA = 15;
int argumentCount;
char** arguments;

template <typename T>
T opt(int index) {
    ensuref(index < argumentCount, "missing generator argument %d", index);
    std::istringstream input(arguments[index]);
    T value;
    input >> value;
    ensuref(input && input.eof(), "invalid generator argument %d: %s",
            index, arguments[index]);
    return value;
}

void checkDelta(int value) {
    ensuref(0 <= value && value <= MAX_DELTA,
            "interval-cost difference %d is outside [0, %d]",
            value, MAX_DELTA);
}

}  // namespace

int main(int argc, char* argv[]) {
    registerGen(argc, argv, 1);
    argumentCount = argc;
    arguments = argv;

    const int n = opt<int>(1);
    const int q = opt<int>(2);
    const long long scale = opt<long long>(3);
    const std::string costMode = opt<std::string>(4);
    const std::string queryMode = opt<std::string>(5);
    ensuref(2 <= n && n <= 2000, "n must be in [2, 2000]");
    ensuref(1 <= q && q <= 100000, "q must be in [1, 100000]");

    ensuref(1 <= scale && scale <= 1000000000LL,
            "scale must be in [1, 1e9]");

    int nextArgument = 6;
    std::vector<std::vector<int>> delta(n, std::vector<int>(n));
    if (costMode == "zero") {
        // Already initialized.
    } else if (costMode == "same") {
        const int value = opt<int>(nextArgument++);
        checkDelta(value);
        for (int r = 1; r < n; ++r) {
            for (int l = 0; l < r; ++l) delta[l][r] = value;
        }
    } else if (costMode == "random") {
        const int low = opt<int>(nextArgument++);
        const int high = opt<int>(nextArgument++);
        checkDelta(low);
        checkDelta(high);
        ensuref(low <= high, "random range is empty");
        for (int r = 1; r < n; ++r) {
            for (int l = 0; l < r; ++l) {
                delta[l][r] = rnd.next(low, high);
            }
        }
    } else if (costMode == "checker") {
        const int low = opt<int>(nextArgument++);
        const int high = opt<int>(nextArgument++);
        checkDelta(low);
        checkDelta(high);
        for (int r = 1; r < n; ++r) {
            for (int l = 0; l < r; ++l) {
                delta[l][r] = (l + r) % 2 == 0 ? low : high;
            }
        }
    } else if (costMode == "bands") {
        const int low = opt<int>(nextArgument++);
        const int high = opt<int>(nextArgument++);
        const int width = opt<int>(nextArgument++);
        checkDelta(low);
        checkDelta(high);
        ensuref(width >= 1, "band width must be positive");
        for (int r = 1; r < n; ++r) {
            for (int l = 0; l < r; ++l) {
                delta[l][r] = ((r - l - 1) / width) % 2 == 0 ? low : high;
            }
        }
    } else if (costMode == "sparse") {
        const int value = opt<int>(nextArgument++);
        const int percent = opt<int>(nextArgument++);
        checkDelta(value);
        ensuref(0 <= percent && percent <= 100,
                "sparse percentage must be in [0, 100]");
        for (int r = 1; r < n; ++r) {
            for (int l = 0; l < r; ++l) {
                delta[l][r] = rnd.next(1, 100) <= percent ? value : 0;
            }
        }
    } else if (costMode == "choice") {
        const int count = opt<int>(nextArgument++);
        ensuref(count >= 1, "choice requires at least one value");
        std::vector<int> values(count);
        for (int& value : values) {
            value = opt<int>(nextArgument++);
            checkDelta(value);
        }
        for (int r = 1; r < n; ++r) {
            for (int l = 0; l < r; ++l) {
                delta[l][r] = values[rnd.next(0, count - 1)];
            }
        }
    } else if (costMode == "gradient") {
        const int maximum = opt<int>(nextArgument++);
        checkDelta(maximum);
        for (int r = 1; r < n; ++r) {
            for (int l = 0; l < r; ++l) {
                delta[l][r] = (3 * l + 5 * r + r - l) % (maximum + 1);
            }
        }
    } else if (costMode == "wide-trap") {
        const int target = opt<int>(nextArgument++);
        ensuref(2 <= target && target + 1 < n,
                "wide-trap target must be in [2, n-2]");
        // The endpoint fans force the neighboring optimal roots far apart.
        // The full interval then has a wide Knuth window whose unique optimum
        // is target - 1, which fixed-count random candidate searches can miss.
        for (int r = 1; r < n; ++r) {
            for (int l = 0; l < r; ++l) {
                if (l == 0) delta[l][r] = r < target ? 2 : 15;
                if (r == n - 1) delta[l][r] = 10;
            }
        }
    } else if (costMode == "ternary-trap") {
        ensuref(n == 10, "ternary-trap requires n = 10");
        const std::vector<std::vector<int>> rows = {
            {},
            {0},
            {14, 15},
            {12, 11, 2},
            {6, 13, 5, 4},
            {0, 1, 1, 1, 2},
            {7, 13, 9, 15, 12, 7},
            {1, 4, 3, 0, 14, 12, 15},
            {10, 2, 5, 9, 2, 4, 4, 11},
            {6, 9, 7, 13, 2, 13, 4, 2, 6}
        };
        for (int r = 1; r < n; ++r) {
            for (int l = 0; l < r; ++l) delta[l][r] = rows[r][l];
        }
    } else {
        quitf(_fail, "unknown cost mode: %s", costMode.c_str());
    }

    std::vector<std::pair<int, int>> queries;
    queries.reserve(q);
    if (queryMode == "all") {
        while (static_cast<int>(queries.size()) < q) {
            for (int length = 2;
                 length <= n && static_cast<int>(queries.size()) < q;
                 ++length) {
                for (int l = 0;
                     l + length <= n && static_cast<int>(queries.size()) < q;
                     ++l) {
                    queries.emplace_back(l, l + length - 1);
                }
            }
        }
    } else if (queryMode == "random") {
        while (static_cast<int>(queries.size()) < q) {
            const int l = rnd.next(0, n - 2);
            const int r = rnd.next(l + 1, n - 1);
            queries.emplace_back(l, r);
        }
    } else if (queryMode == "short") {
        while (static_cast<int>(queries.size()) < q) {
            const int length = rnd.next(2, std::min(n, 9));
            const int l = rnd.next(0, n - length);
            queries.emplace_back(l, l + length - 1);
        }
    } else if (queryMode == "full") {
        queries.assign(q, {0, n - 1});
    } else if (queryMode == "boundary") {
        for (int i = 0; i < q; ++i) {
            if (i % 2 == 0) {
                queries.emplace_back(0, rnd.next(1, n - 1));
            } else {
                queries.emplace_back(rnd.next(0, n - 2), n - 1);
            }
        }
    } else if (queryMode == "nested") {
        const int layers = (n + 1) / 2;
        for (int i = 0; i < q; ++i) {
            int l = i % layers;
            int r = n - 1 - l;
            if (l == r) --l;
            queries.emplace_back(l, r);
        }
    } else if (queryMode == "mixed") {
        for (int i = 0; i < q; ++i) {
            if (i % 6 == 0) {
                queries.emplace_back(0, n - 1);
            } else if (i % 6 == 1) {
                const int l = rnd.next(0, n - 2);
                queries.emplace_back(l, l + 1);
            } else if (i % 6 == 2) {
                const int l = rnd.next(0, n - 2);
                queries.emplace_back(l, rnd.next(l + 1, n - 1));
            } else if (i % 6 == 3) {
                queries.emplace_back(0, rnd.next(1, n - 1));
            } else if (i % 6 == 4) {
                queries.emplace_back(rnd.next(0, n - 2), n - 1);
            } else {
                const int layer = (i / 6) % ((n + 1) / 2);
                int l = layer;
                int r = n - 1 - layer;
                if (l == r) --l;
                queries.emplace_back(l, r);
            }
        }
    } else if (queryMode == "fixed") {
        const int l = opt<int>(nextArgument++);
        const int r = opt<int>(nextArgument++);
        ensuref(0 <= l && l < r && r < n,
                "fixed query must satisfy 0 <= l < r < n");
        queries.assign(q, {l, r});
    } else {
        quitf(_fail, "unknown query mode: %s", queryMode.c_str());
    }

    std::cout << n << ' ' << q << ' ' << scale << '\n';
    for (int r = 1; r < n; ++r) {
        for (int l = 0; l < r; ++l) {
            if (l) std::cout << ' ';
            std::cout << delta[l][r];
        }
        std::cout << '\n';
    }
    for (const auto& interval : queries) {
        std::cout << interval.first << ' ' << interval.second << '\n';
    }
}
