#include "testlib.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace {

constexpr long long LIMIT = 1000000000000000000LL;
int argumentCount;
char** arguments;

template <typename T>
T opt(int index) {
    ensuref(index < argumentCount, "missing generator argument %d", index);
    istringstream input(arguments[index]);
    T value;
    input >> value;
    ensuref(input && input.eof(), "invalid generator argument %d: %s", index, arguments[index]);
    return value;
}

void requireValue(long long value, const string& name) {
    ensuref(0 <= value && value <= LIMIT, "%s must be in [0, 10^18]", name.c_str());
}

vector<long long> makeSequence(int n, long long start, long long step) {
    vector<long long> values;
    values.reserve(n);
    for (int i = 0; i < n; ++i) {
        __int128 value = static_cast<__int128>(start) + static_cast<__int128>(i) * step;
        ensuref(0 <= value && value <= LIMIT, "sequence value is outside [0, 10^18]");
        values.push_back(static_cast<long long>(value));
    }
    return values;
}

}  // namespace

int main(int argc, char* argv[]) {
    registerGen(argc, argv, 1);
    argumentCount = argc;
    arguments = argv;

    const int n = opt<int>(1);
    const int k = opt<int>(2);
    const string mode = opt<string>(3);
    ensuref(1 <= n && n <= 2000, "n must be in [1, 2000]");
    ensuref(1 <= k && k <= n, "k must be in [1, n]");

    vector<long long> values;
    values.reserve(n);

    if (mode == "same") {
        const long long value = opt<long long>(4);
        requireValue(value, "value");
        values.assign(n, value);
    } else if (mode == "uniform") {
        const long long low = opt<long long>(4);
        const long long high = opt<long long>(5);
        requireValue(low, "low");
        requireValue(high, "high");
        ensuref(low <= high, "low must not exceed high");
        for (int i = 0; i < n; ++i) values.push_back(rnd.next(low, high));
    } else if (mode == "increasing") {
        const long long start = opt<long long>(4);
        const long long step = opt<long long>(5);
        ensuref(step >= 0, "increasing step must be non-negative");
        values = makeSequence(n, start, step);
    } else if (mode == "decreasing") {
        const long long start = opt<long long>(4);
        const long long step = opt<long long>(5);
        ensuref(step >= 0, "decreasing step must be non-negative");
        values = makeSequence(n, start, -step);
    } else if (mode == "progression") {
        const long long start = opt<long long>(4);
        const long long step = opt<long long>(5);
        values = makeSequence(n, start, step);
        shuffle(values.begin(), values.end());
    } else if (mode == "choice") {
        const int count = opt<int>(4);
        ensuref(count >= 1, "choice mode needs at least one candidate");
        vector<long long> candidates(count);
        for (int i = 0; i < count; ++i) {
            candidates[i] = opt<long long>(5 + i);
            requireValue(candidates[i], "candidate");
        }
        for (int i = 0; i < n; ++i) values.push_back(candidates[rnd.next(count)]);
    } else if (mode == "groups") {
        const int groupCount = opt<int>(4);
        ensuref(groupCount >= 1, "groups mode needs at least one group");
        int total = 0;
        for (int i = 0; i < groupCount; ++i) {
            const long long value = opt<long long>(5 + 2 * i);
            const int count = opt<int>(6 + 2 * i);
            requireValue(value, "group value");
            ensuref(count >= 0, "group size must be non-negative");
            total += count;
            values.insert(values.end(), count, value);
        }
        ensuref(total == n, "group sizes sum to %d instead of n = %d", total, n);
        shuffle(values.begin(), values.end());
    } else {
        quitf(_fail, "unknown generation mode: %s", mode.c_str());
    }

    cout << n << ' ' << k << '\n';
    println(values);
}
