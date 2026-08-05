#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;
    std::vector<long long> times(n);
    std::vector<int> frequency(43200, 0);
    for (long long& value : times) {
        std::cin >> value;
        ++frequency[static_cast<int>(value)];
    }

    std::sort(times.begin(), times.end());
    std::vector<long long> prefix(n + 1, 0);
    for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + times[i];

    std::vector<std::pair<int, int>> candidates;
    for (int value = 0; value < 43200; ++value) {
        if (frequency[value] != 0) {
            candidates.emplace_back(-frequency[value], value);
        }
    }
    std::sort(candidates.begin(), candidates.end());

    // Deliberately arbitrary pruning: even if every possible time occurs,
    // the last two candidates are never inspected.
    constexpr int MAGIC = 43198;
    const int inspected = std::min<int>(MAGIC, candidates.size());
    long long answer = (1LL << 62);
    for (int index = 0; index < inspected; ++index) {
        const int target = candidates[index].second;
        const int split = static_cast<int>(
            std::upper_bound(times.begin(), times.end(), target) -
            times.begin());
        const long long cost =
            1LL * split * target - prefix[split] +
            1LL * (n - split) * (43200 + target) -
            (prefix[n] - prefix[split]);
        answer = std::min(answer, cost);
    }

    std::cout << answer << '\n';
}
