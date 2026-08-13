#include "testlib.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr long long MAX_DELTA = 3e15;
constexpr int MATRIX_SIZE = 2000;
constexpr char TESTCASE_MAGIC[8] = {'C', 'A', 'K', 'E', '4', 'B', 'I', 'N'};
using Matrix =
    std::array<std::array<long long, MATRIX_SIZE>, MATRIX_SIZE>;
Matrix w;
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

void checkDelta(long long value) {
    ensuref(0 <= value && value <= MAX_DELTA,
            "interval-cost difference %lld is outside [0, %lld]",
            value, MAX_DELTA);
}

template <typename T>
void writeBinary(const T& value) {
    std::cout.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void writeBytes(const void* data, std::size_t size) {
    std::cout.write(static_cast<const char*>(data),
                    static_cast<std::streamsize>(size));
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
    std::vector<std::vector<long long>> delta(
        n, std::vector<long long>(n));
    if (costMode == "example") {
        ensuref(n == 4, "example cost mode requires n = 4");
        delta[1][2] = 1;
        delta[2][3] = 15;
    } else if (costMode == "zero") {
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
    } else if (costMode == "zero-edge-trap") {
        ensuref(n >= 3, "zero-edge-trap requires n >= 3");
        // w[0][1] = 0 and w[1][2] = 1.  For [0,2], the unique optimum
        // is therefore k = 1; forcing every adjacent cost positive changes it.
        delta[0][1] = -1;
    } else if (costMode == "huge-trap") {
        ensuref(n >= 4, "huge-trap requires n >= 4");
        // Clamping the two 1e14 entries to 1e9 changes opt[0][3]
        // from 0 to 1, while all expanded costs remain below 3e16.
        delta[0][1] = 100000000000000LL;
        delta[0][2] = 100000000000000LL;
        delta[1][2] = 1000000000LL;
        delta[2][3] = 1000000000LL;
    } else if (costMode == "double-trap") {
        ensuref(n >= 3, "double-trap requires n >= 3");
        // For [0,2], the two candidates differ by exactly one near 1.5e15.
        // IEEE double rounds them to the same value and picks the wrong root.
        delta[0][1] = 1499999999999998LL;
        delta[1][2] = 1499999999999999LL;
        for (int r = 3; r < n; ++r) delta[r - 1][r] = -1;
    } else {
        quitf(_fail, "unknown cost mode: %s", costMode.c_str());
    }

    std::vector<std::pair<int, int>> queries;
    queries.reserve(q);
    if (queryMode == "example") {
        ensuref(n == 4 && q == 3,
                "example query mode requires n = 4 and q = 3");
        queries = {{0, 3}, {0, 1}, {1, 3}};
    } else if (queryMode == "all") {
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

    for (int r = 1; r < n; ++r) {
        long long extensionCost = 1;
        for (int l = r - 1; l >= 0; --l) {
            extensionCost += delta[l][r];
            const __int128 current =
                static_cast<__int128>(w[l][r - 1]) +
                static_cast<__int128>(extensionCost) * scale;
            ensuref(current <= std::numeric_limits<long long>::max(),
                    "expanded interval cost overflows int64");
            w[l][r] = static_cast<long long>(current);
        }
    }

    const auto index = [n](int l, int r) {
        return static_cast<std::size_t>(l) * n + r;
    };
    std::vector<long long> dp(static_cast<std::size_t>(n) * n);
    std::vector<int> optimalCut(static_cast<std::size_t>(n) * n);
    for (int i = 0; i < n; ++i) optimalCut[index(i, i)] = i;
    for (int length = 2; length <= n; ++length) {
        for (int l = 0; l + length <= n; ++l) {
            const int r = l + length - 1;
            const int low = std::max(optimalCut[index(l, r - 1)], l);
            const int high =
                std::min(optimalCut[index(l + 1, r)], r - 1);
            long long best = std::numeric_limits<long long>::max();
            int bestCut = low;
            for (int k = low; k <= high; ++k) {
                const long long candidate =
                    dp[index(l, k)] + dp[index(k + 1, r)];
                if (candidate < best) {
                    best = candidate;
                    bestCut = k;
                }
            }
            ensuref(best <= std::numeric_limits<long long>::max() - w[l][r],
                    "optimal cost overflows int64");
            dp[index(l, r)] = best + w[l][r];
            optimalCut[index(l, r)] = bestCut;
        }
    }

    static_assert(sizeof(long long) == sizeof(std::int64_t),
                  "generator requires 64-bit long long");
    writeBytes(TESTCASE_MAGIC, sizeof(TESTCASE_MAGIC));
    const std::uint32_t binaryN = static_cast<std::uint32_t>(n);
    const std::uint32_t binaryQ = static_cast<std::uint32_t>(q);
    writeBinary(binaryN);
    writeBinary(binaryQ);
    writeBytes(w.data(), static_cast<std::size_t>(n) * sizeof(w[0]));
    writeBytes(dp.data(), dp.size() * sizeof(dp[0]));
    for (const auto& interval : queries) {
        const std::int32_t l = interval.first;
        const std::int32_t r = interval.second;
        writeBinary(l);
        writeBinary(r);
    }
    ensuref(std::cout.good(), "failed to write binary testcase");
}
