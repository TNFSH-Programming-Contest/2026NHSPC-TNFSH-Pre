#include "testlib.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace {

constexpr int PERIOD = 43200;
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


int normalize(long long value) {
    value %= PERIOD;
    if (value < 0) value += PERIOD;
    return static_cast<int>(value);
}

void requireTime(int value, const string& name) {
    ensuref(0 <= value && value < PERIOD, "%s must be in [0, %d)", name.c_str(), PERIOD);
}

}  // namespace

int main(int argc, char* argv[]) {
    registerGen(argc, argv, 1);

    argumentCount = argc;
    arguments = argv;
    const int n = opt<int>(1);
    const string mode = opt<string>(2);
    ensuref(1 <= n && n <= 200000, "n must be in [1, 200000]");

    vector<int> times;
    times.reserve(n);

    if (mode == "same") {
        const int value = opt<int>(3);
        requireTime(value, "value");
        times.assign(n, value);
    } else if (mode == "uniform") {
        const int low = opt<int>(3);
        const int high = opt<int>(4);
        requireTime(low, "low");
        requireTime(high, "high");
        ensuref(low <= high, "low must not exceed high");
        for (int i = 0; i < n; ++i) times.push_back(rnd.next(low, high));
    } else if (mode == "pair") {
        ensuref(n == 2, "pair mode requires n = 2");
        const int first = opt<int>(3);
        const int second = opt<int>(4);
        requireTime(first, "first");
        requireTime(second, "second");
        times = {first, second};
    } else if (mode == "progression") {
        const int start = opt<int>(3);
        const int step = opt<int>(4);
        requireTime(start, "start");
        for (int i = 0; i < n; ++i) {
            times.push_back(normalize(start + 1LL * i * step));
        }
        shuffle(times.begin(), times.end());
    } else if (mode == "cluster") {
        const int center = opt<int>(3);
        const int radius = opt<int>(4);
        requireTime(center, "center");
        ensuref(0 <= radius && radius < PERIOD, "radius must be in [0, 43200)");
        for (int i = 0; i < n; ++i) {
            times.push_back(normalize(center + rnd.next(-radius, radius)));
        }
    } else if (mode == "scatter") {
        const int offset = opt<int>(3);
        requireTime(offset, "offset");
        ensuref(n <= PERIOD, "scatter mode requires n <= 43200");
        for (int i = 0; i < n; ++i) {
            const int left = static_cast<int>(1LL * i * PERIOD / n);
            const int right =
                static_cast<int>(1LL * (i + 1) * PERIOD / n) - 1;
            times.push_back(normalize(offset + rnd.next(left, right)));
        }
        shuffle(times.begin(), times.end());
    } else if (mode == "choice") {
        const int count = opt<int>(3);
        ensuref(count >= 1, "choice mode needs at least one candidate");
        vector<int> candidates(count);
        for (int i = 0; i < count; ++i) {
            candidates[i] = opt<int>(4 + i);
            requireTime(candidates[i], "candidate");
        }
        for (int i = 0; i < n; ++i) times.push_back(candidates[rnd.next(count)]);
    } else if (mode == "groups") {
        const int groupCount = opt<int>(3);
        ensuref(groupCount >= 1, "groups mode needs at least one group");
        int total = 0;
        for (int i = 0; i < groupCount; ++i) {
            const int value = opt<int>(4 + 2 * i);
            const int count = opt<int>(5 + 2 * i);
            requireTime(value, "group value");
            ensuref(count >= 0, "group size must be non-negative");
            total += count;
            times.insert(times.end(), count, value);
        }
        ensuref(total == n, "group sizes sum to %d instead of n = %d", total, n);
        shuffle(times.begin(), times.end());
    } else if (mode == "ranktrap") {
        const int start = opt<int>(3);
        const int decoys = opt<int>(4);
        const int decoyFrequency = opt<int>(5);
        const int targetFrequency = opt<int>(6);
        requireTime(start, "start");
        ensuref(1 <= decoys && decoys < PERIOD,
                "decoys must be in [1, 43200)");
        ensuref(1 <= targetFrequency &&
                    targetFrequency < decoyFrequency,
                "frequencies must satisfy 1 <= target < decoy");
        const long long expectedN =
            1LL * decoys * decoyFrequency + targetFrequency;
        ensuref(expectedN == n,
                "ranktrap frequencies produce %lld values instead of n = %d",
                expectedN, n);
        ensuref(n < 1LL * PERIOD * targetFrequency,
                "target is not guaranteed to improve the circular cost");

        for (int offset = 0; offset < decoys; ++offset) {
            const int value = normalize(start + offset);
            times.insert(times.end(), decoyFrequency, value);
        }
        const int target = normalize(start + decoys);
        times.insert(times.end(), targetFrequency, target);
        shuffle(times.begin(), times.end());
    } else {
        quitf(_fail, "unknown generation mode: %s", mode.c_str());
    }

    cout << n << '\n';
    println(times);
}
