#include "testlib.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr long long MAX_DELTA = 3000000000000000LL;
constexpr long long MAX_COST = 3000000000000000LL;
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

std::uint64_t splitmix64(std::uint64_t value) {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

long double deterministicUnit(int l, int r, int seed) {
    const std::uint64_t value = splitmix64(
        static_cast<std::uint64_t>(l) * 2003 + r +
        static_cast<std::uint64_t>(seed) * 1000000007ULL);
    return (value >> 11) *
        (1.0L / (std::uint64_t{1} << 53));
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

    const std::string subtask = opt<std::string>(1);
    const int n = opt<int>(2);
    const int q = opt<int>(3);
    const long long scale = opt<long long>(4);
    const std::string costMode = opt<std::string>(5);
    const std::string queryMode = opt<std::string>(6);
    ensuref(2 <= n && n <= 2000, "n must be in [2, 2000]");
    ensuref(1 <= q && q <= 100000, "q must be in [1, 100000]");

    ensuref(1 <= scale && scale <= 1000000000LL,
            "scale must be in [1, 1e9]");

    int nextArgument = 7;
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
    } else if (costMode == "entropy") {
        ensuref(n == 2000, "entropy cost mode requires n = 2000");
        ensuref(scale == 1, "entropy cost mode requires scale = 1");

        // Let F(x) = (2 / (x + 2))^1.6 and G(r) = F(n - 1 - r).
        // The real-valued rank-one Monge array
        //
        //   W[l][r] = C * G(r) * (F(l) - F(r))
        //
        // makes the optimal-root diagonals spread almost uniformly from 0
        // to n - 2.  Consequently, the gaps between consecutive roots are
        // close to the balanced gaps from the mixed-radix upper bound.
        //
        // Store rounded nonnegative cross-differences instead of rounding W
        // itself.  This preserves the Monge inequalities exactly even after
        // converting the construction to integers.
        constexpr long double EXPONENT = 1.6L;
        constexpr long double OFFSET = 2.0L;
        constexpr long double AMPLITUDE = 2900000000000000.0L;
        std::vector<long double> profile(n);
        for (int x = 0; x < n; ++x) {
            profile[x] =
                std::pow(OFFSET / (static_cast<long double>(x) + OFFSET),
                         EXPONENT);
        }

        for (int r = 1; r < n; ++r) {
            const long double right = profile[n - 1 - r];
            const long double previousRight = profile[n - r];

            const long long adjacent = std::llround(
                AMPLITUDE * right * (profile[r - 1] - profile[r]));
            ensuref(adjacent >= 0, "negative entropy adjacent cost");
            delta[r - 1][r] = adjacent - 1;

            for (int l = r - 2; l >= 0; --l) {
                const long long cross = std::llround(
                    AMPLITUDE * (profile[l] - profile[l + 1]) *
                    (right - previousRight));
                checkDelta(cross);
                delta[l][r] = cross;
            }
        }
    } else if (costMode == "arithmetic-entropy") {
        ensuref(n == 2000,
                "arithmetic-entropy cost mode requires n = 2000");
        ensuref(scale == 1,
                "arithmetic-entropy cost mode requires scale = 1");

        // Keep the large, nearly balanced Knuth windows from entropy, but
        // perturb every nonnegative Monge cross-difference independently.
        // The perturbation removes the strong correlation between the chosen
        // digit and the (radix, predicted digit) contexts used by adaptive
        // arithmetic coders without invalidating the quadrangle inequality.
        constexpr long double EXPONENT = 1.6L;
        constexpr long double OFFSET = 2.0L;
        constexpr long double AMPLITUDE = 2900000000000000.0L;
        constexpr long double MULTIPLICATIVE_NOISE = 0.0003L;
        constexpr int HASH_SEED = 7;
        constexpr int NOISE_BY_SPAN[10] = {
            2, 10, 30, 30, 30, 30, 3, 6, 4, 4
        };
        constexpr int BIAS[8][8] = {
            {-125, 125, 0, 1250, 2500, -1250, 0, 0},
            {-4, 0, -1, 0, 2500, 0, 0, 0},
            {0, 0, 1, 0, 2500, 0, 0, 0},
            {0, 0, 0, 0, 2500, 0, 0, 0},
            {0, 4, 0, -75, 0, 0, 0, 0},
            {0, 6, 0, 0, 0, 0, 0, 0},
            {0, 50, 0, 0, 0, 0, 0, 0},
            {75, 0, 0, 0, 0, 0, 0, 0}
        };
        std::vector<long double> profile(n);
        for (int x = 0; x < n; ++x) {
            profile[x] =
                std::pow(OFFSET / (static_cast<long double>(x) + OFFSET),
                         EXPONENT);
        }

        for (int r = 1; r < n; ++r) {
            const long double right = profile[n - 1 - r];
            const long double previousRight = profile[n - r];
            const long double adjacentBase =
                AMPLITUDE * right * (profile[r - 1] - profile[r]);
            const int leftBin = (r - 1) * 8 / n;
            const long long adjacent = std::max<long long>(
                0, std::llround(adjacentBase *
                    (1 + MULTIPLICATIVE_NOISE *
                        (2 * deterministicUnit(r - 1, r, HASH_SEED) - 1)) +
                    NOISE_BY_SPAN[0] *
                        (2 * deterministicUnit(r, r, HASH_SEED) - 1) +
                    BIAS[leftBin][0]));
            delta[r - 1][r] = adjacent - 1;

            for (int l = r - 2; l >= 0; --l) {
                const int span = r - l;
                const int spanNoiseBin =
                    std::min(9, span * 10 / n);
                const int spanBiasBin =
                    std::min(7, span * 8 / n);
                const long double crossBase =
                    AMPLITUDE * (profile[l] - profile[l + 1]) *
                    (right - previousRight);
                const long long cross = std::max<long long>(
                    0, std::llround(crossBase *
                        (1 + MULTIPLICATIVE_NOISE *
                            (2 * deterministicUnit(
                                l + 17, r, HASH_SEED) - 1)) +
                        NOISE_BY_SPAN[spanNoiseBin] *
                            (2 * deterministicUnit(l, r, HASH_SEED) - 1) +
                        BIAS[l * 8 / n][spanBiasBin]));
                checkDelta(cross);
                delta[l][r] = cross;
            }
        }
    } else if (costMode == "profile-entropy") {
        ensuref(n == 2000,
                "profile-entropy cost mode requires n = 2000");
        ensuref(scale == 1,
                "profile-entropy cost mode requires scale = 1");
        const long double leftExponent = opt<long double>(nextArgument++);
        const long double leftOffset = opt<long double>(nextArgument++);
        const long double rightExponent = opt<long double>(nextArgument++);
        const long double rightOffset = opt<long double>(nextArgument++);
        const int noise = opt<int>(nextArgument++);
        const int hashSeed = opt<int>(nextArgument++);
        ensuref(leftExponent > 0 && rightExponent > 0,
                "profile exponents must be positive");
        ensuref(leftOffset > 0 && rightOffset > 0,
                "profile offsets must be positive");
        ensuref(0 <= noise && noise <= 1000000,
                "profile noise must be in [0, 1e6]");

        constexpr long double AMPLITUDE = 2900000000000000.0L;
        std::vector<long double> left(n), right(n);
        for (int x = 0; x < n; ++x) {
            left[x] = std::pow(
                leftOffset /
                    (static_cast<long double>(x) + leftOffset),
                leftExponent);
            right[x] = std::pow(
                rightOffset /
                    (static_cast<long double>(x) + rightOffset),
                rightExponent);
        }
        for (int r = 1; r < n; ++r) {
            const long double adjacentBase =
                AMPLITUDE * right[n - 1 - r] *
                (left[r - 1] - left[r]);
            const long long adjacent = std::max<long long>(
                0, std::llround(adjacentBase + noise *
                    (2 * deterministicUnit(r, r, hashSeed) - 1)));
            delta[r - 1][r] = adjacent - 1;
            for (int l = r - 2; l >= 0; --l) {
                const long double crossBase =
                    AMPLITUDE * (left[l] - left[l + 1]) *
                    (right[n - 1 - r] - right[n - r]);
                const long long cross = std::max<long long>(
                    0, std::llround(crossBase + noise *
                        (2 * deterministicUnit(l, r, hashSeed) - 1)));
                checkDelta(cross);
                delta[l][r] = cross;
            }
        }
    } else if (costMode == "truncated-entropy") {
        ensuref(n == 2000,
                "truncated-entropy cost mode requires n = 2000");
        ensuref(scale == 1,
                "truncated-entropy cost mode requires scale = 1");

        // A positive mixture of rank-one Monge arrays.  Unlike entropy,
        // which balances every gap, these three components deliberately make
        // the gaps uneven and move the selected digit into the long-code part
        // of truncated binary coding whenever that does not destroy the next
        // diagonal's gaps.
        constexpr int COMPONENTS = 3;
        constexpr long double AMPLITUDE = 2900000000000000.0L;
        constexpr long double ALPHA = 0.748454L;
        constexpr long double BETA = 0.305952L;
        const long double leftExponent[COMPONENTS] = {
            1.74433L, 1.95069L, 0.751814L
        };
        const long double leftOffset[COMPONENTS] = {
            126.223L, 1.69341L, 32.6169L
        };
        const long double rightExponent[COMPONENTS] = {
            3.02251L, 1.34474L, 3.00615L
        };
        const long double rightOffset[COMPONENTS] = {
            3.46462L, 11.926L, 1.64061L
        };
        const long double weight[COMPONENTS] = {
            (1 - BETA) * ALPHA,
            (1 - BETA) * (1 - ALPHA),
            BETA
        };

        std::vector<std::vector<long double>> left(
            COMPONENTS, std::vector<long double>(n));
        std::vector<std::vector<long double>> right(
            COMPONENTS, std::vector<long double>(n));
        for (int component = 0; component < COMPONENTS; ++component) {
            for (int x = 0; x < n; ++x) {
                left[component][x] = std::pow(
                    leftOffset[component] /
                        (static_cast<long double>(x) +
                         leftOffset[component]),
                    leftExponent[component]);
                right[component][x] = std::pow(
                    rightOffset[component] /
                        (static_cast<long double>(x) +
                         rightOffset[component]),
                    rightExponent[component]);
            }
        }

        for (int r = 1; r < n; ++r) {
            long double adjacent = 0;
            for (int component = 0; component < COMPONENTS; ++component) {
                adjacent += weight[component] *
                    right[component][n - 1 - r] *
                    (left[component][r - 1] - left[component][r]);
            }
            const long long adjacentCost =
                std::llround(AMPLITUDE * adjacent);
            ensuref(adjacentCost >= 0,
                    "negative truncated-entropy adjacent cost");
            delta[r - 1][r] = adjacentCost - 1;

            for (int l = r - 2; l >= 0; --l) {
                long double cross = 0;
                for (int component = 0; component < COMPONENTS;
                     ++component) {
                    cross += weight[component] *
                        (left[component][l] - left[component][l + 1]) *
                        (right[component][n - 1 - r] -
                         right[component][n - r]);
                }
                const long long roundedCross =
                    std::llround(AMPLITUDE * cross);
                checkDelta(roundedCross);
                delta[l][r] = roundedCross;
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

    // Some high-scale modes can produce a valid Monge array whose entries
    // exceed the validator's W bound.  Find one global divisor first; applying
    // it to every extension preserves their ordering and all monotonicity
    // constraints.  Existing cases already within the bound use divisor 1.
    std::vector<__int128> rawCost(n);
    __int128 maximumRawCost = 0;
    for (int r = 1; r < n; ++r) {
        long long extensionCost = 1;
        for (int l = r - 1; l >= 0; --l) {
            extensionCost += delta[l][r];
            rawCost[l] +=
                static_cast<__int128>(extensionCost) * scale;
            maximumRawCost = std::max(maximumRawCost, rawCost[l]);
        }
    }
    const __int128 costDivisor = std::max<__int128>(
        1, (maximumRawCost + MAX_COST - 1) / MAX_COST);

    for (int r = 1; r < n; ++r) {
        long long extensionCost = 1;
        for (int l = r - 1; l >= 0; --l) {
            extensionCost += delta[l][r];
            const __int128 current =
                static_cast<__int128>(w[l][r - 1]) +
                static_cast<__int128>(extensionCost) * scale / costDivisor;
            ensuref(current <= MAX_COST,
                    "normalized interval cost exceeds W limit");
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

    if (costMode == "entropy" || costMode == "arithmetic-entropy" ||
        costMode == "profile-entropy") {
        long long encodedBits = 0;
        for (int d = 1; d < n; ++d) {
            long double diagonalBits = 0;
            for (int l = 0; l + d < n; ++l) {
                const int radix =
                    optimalCut[index(l + 1, l + d)] -
                    optimalCut[index(l, l + d - 1)] + 1;
                ensuref(radix >= 1, "entropy construction broke Knuth monotonicity");
                diagonalBits += std::log2(static_cast<long double>(radix));
            }
            encodedBits += static_cast<long long>(
                std::ceil(diagonalBits - 1e-12L));
        }
        const long long minimumBits = costMode == "entropy"
            ? 2700000 : 2600000;
        ensuref(encodedBits >= minimumBits,
                "%s construction only produced %lld mixed-radix bits",
                costMode.c_str(), encodedBits);
    }

    if (costMode == "truncated-entropy") {
        long long encodedBits = 0;
        for (int d = 2; d < n; ++d) {
            for (int l = 0; l + d < n; ++l) {
                const int low = optimalCut[index(l, l + d - 1)];
                const int high = optimalCut[index(l + 1, l + d)];
                const int radix = high - low + 1;
                const int symbol = optimalCut[index(l, l + d)] - low;
                const int shortBits =
                    31 - __builtin_clz(static_cast<unsigned>(radix));
                if (radix == (1 << shortBits)) {
                    encodedBits += shortBits;
                } else {
                    const int shortCodes = (1 << (shortBits + 1)) - radix;
                    encodedBits += shortBits + (symbol >= shortCodes);
                }
            }
        }
        ensuref(encodedBits >= 2900000,
                "truncated-entropy construction only produced %lld bits",
                encodedBits);
    }

    static_assert(sizeof(long long) == sizeof(std::int64_t),
                  "generator requires 64-bit long long");
    writeBytes(TESTCASE_MAGIC, sizeof(TESTCASE_MAGIC));
    const std::uint32_t binaryN = static_cast<std::uint32_t>(n);
    const std::uint32_t binaryQ = static_cast<std::uint32_t>(q);
    writeBinary(binaryN);
    writeBinary(binaryQ);

    const std::uint32_t binarySubtaskLength = static_cast<std::uint32_t>(subtask.size());
    writeBinary(binarySubtaskLength);
    writeBytes(subtask.data(), subtask.size());

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
