#include "testlib.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>

namespace {

constexpr long long MAX_COST = 30000000000000000LL;
constexpr int MATRIX_SIZE = 2000;
constexpr char TESTCASE_MAGIC[8] = {'C', 'A', 'K', 'E', '4', 'B', 'I', 'N'};
using Matrix =
    std::array<std::array<long long, MATRIX_SIZE>, MATRIX_SIZE>;

Matrix w;

bool readBytes(void* destination, std::size_t size) {
    char* output = static_cast<char*>(destination);
    while (size != 0) {
        const std::size_t received = std::fread(output, 1, size, stdin);
        if (received == 0) return false;
        output += received;
        size -= received;
    }
    return true;
}

template <typename T>
bool readBinary(T& value) {
    return readBytes(&value, sizeof(value));
}

}  // namespace

int main(int argc, char* argv[]) {
    registerValidation(argc, argv);
    ensuref(argc == 3, "validator requires maxN and maxQ");

    const int maxN = std::stoi(argv[1]);
    const int maxQ = std::stoi(argv[2]);
    ensuref(2 <= maxN && maxN <= MATRIX_SIZE,
            "maxN must be in [2, 2000]");
    ensuref(1 <= maxQ && maxQ <= 100000,
            "maxQ must be in [1, 100000]");

    static_assert(sizeof(long long) == sizeof(std::int64_t),
                  "validator requires 64-bit long long");

    char magic[sizeof(TESTCASE_MAGIC)];
    std::uint32_t binaryN;
    std::uint32_t binaryQ;
    std::uint32_t binarySubtaskLength;
    ensuref(readBytes(magic, sizeof(magic)) &&
                std::memcmp(magic, TESTCASE_MAGIC, sizeof(magic)) == 0,
            "invalid binary testcase magic");
    ensuref(readBinary(binaryN) && readBinary(binaryQ) && readBinary(binarySubtaskLength),
            "truncated binary testcase header");
    ensuref(binarySubtaskLength != 0,
            "binary testcase header has zero subtask length");
    char *subtaskName = new char[binarySubtaskLength + 1];
    ensuref(readBytes(subtaskName, binarySubtaskLength),
            "truncated binary subtask name");
    delete[] subtaskName;
    ensuref(2 <= binaryN && binaryN <= static_cast<std::uint32_t>(maxN),
            "n is outside this subtask's range");
    ensuref(1 <= binaryQ && binaryQ <= static_cast<std::uint32_t>(maxQ),
            "Q is outside this subtask's range");
    const int n = static_cast<int>(binaryN);
    const int q = static_cast<int>(binaryQ);

    ensuref(readBytes(w.data(), static_cast<std::size_t>(n) * sizeof(w[0])),
            "truncated binary W array");

    for (int l = 0; l < n; ++l) {
        for (int r = 0; r < MATRIX_SIZE; ++r) {
            if (r < l || r >= n) {
                ensuref(w[l][r] == 0,
                        "unused W[%d][%d] must be zero", l, r);
            } else {
                ensuref(0 <= w[l][r] && w[l][r] <= MAX_COST,
                        "W[%d][%d] is outside the allowed range", l, r);
            }
        }
        ensuref(w[l][l] == 0, "W[%d][%d] must be zero", l, l);
    }

    for (int r = 1; r < n; ++r) {
        for (int l = r - 1; l >= 0; --l) {
            const long long extension = w[l][r] - w[l][r - 1];
            ensuref(extension >= 0,
                    "W is not monotone when extending [%d,%d]", l, r - 1);
            if (l + 1 < r) {
                const long long shorterExtension =
                    w[l + 1][r] - w[l + 1][r - 1];
                ensuref(extension >= shorterExtension,
                        "W has decreasing marginal extension at [%d,%d]",
                        l, r);
                ensuref(w[l][r] >= w[l + 1][r],
                        "W is not monotone under interval containment");
            }
        }
    }

    const std::size_t tableSize = static_cast<std::size_t>(n) * n;
    std::vector<long long> suppliedDp(tableSize);
    ensuref(readBytes(suppliedDp.data(),
                      suppliedDp.size() * sizeof(suppliedDp[0])),
            "truncated binary DP array");

    const auto index = [n](int l, int r) {
        return static_cast<std::size_t>(l) * n + r;
    };
    std::vector<long long> expectedDp(tableSize);
    std::vector<int> optimalCut(tableSize);
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
                    expectedDp[index(l, k)] +
                    expectedDp[index(k + 1, r)];
                if (candidate < best) {
                    best = candidate;
                    bestCut = k;
                }
            }
            const __int128 current =
                static_cast<__int128>(best) + w[l][r];
            ensuref(current <= std::numeric_limits<long long>::max(),
                    "optimal DP cost overflows int64 at [%d,%d]", l, r);
            expectedDp[index(l, r)] = static_cast<long long>(current);
            optimalCut[index(l, r)] = bestCut;
        }
    }
    ensuref(suppliedDp == expectedDp,
            "supplied binary DP array does not match W");

    for (int i = 0; i < q; ++i) {
        std::int32_t l;
        std::int32_t r;
        ensuref(readBinary(l) && readBinary(r),
                "truncated binary query list");
        ensuref(0 <= l && l < r && r < n,
                "query %d is outside 0 <= l < r < n", i + 1);
    }
    inf.readEof();
}
