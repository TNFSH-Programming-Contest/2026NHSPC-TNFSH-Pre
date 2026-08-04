#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

namespace {

constexpr int PERIOD = 43200;

}  // namespace

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;
    std::vector<long long> times(n);
    for (long long& value : times) std::cin >> value;
    std::sort(times.begin(), times.end());

    std::vector<long long> prefix(n + 1);
    for (int i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + times[i];
    }

    const auto cost = [&](int target) {
        const int split = static_cast<int>(
            std::upper_bound(times.begin(), times.end(), target) -
            times.begin());
        const long long left =
            1LL * split * target - prefix[split];
        const long long right =
            1LL * (n - split) * (PERIOD + target) -
            (prefix[n] - prefix[split]);
        return left + right;
    };

    std::mt19937 random(0x6767);
    std::uniform_real_distribution<double> probability(0.0, 1.0);
    long long answer = (1LL << 62);

    constexpr int RESTARTS = 8;
    constexpr int STEPS = 3000;
    for (int restart = 0; restart < RESTARTS; ++restart) {
        int current = static_cast<int>(random() % PERIOD);
        long long currentCost = cost(current);
        answer = std::min(answer, currentCost);

        double temperature = PERIOD / 2.0;
        for (int step = 0; step < STEPS; ++step) {
            const int radius = std::max(1, static_cast<int>(temperature));
            const int offset =
                static_cast<int>(random() % (2 * radius + 1)) - radius;
            int next = (current + offset) % PERIOD;
            if (next < 0) next += PERIOD;
            const long long nextCost = cost(next);

            const double costTemperature =
                std::max(1.0, temperature * n);
            if (nextCost <= currentCost ||
                probability(random) <
                    std::exp((currentCost - nextCost) / costTemperature)) {
                current = next;
                currentCost = nextCost;
            }
            answer = std::min(answer, currentCost);
            temperature *= 0.997;
        }
    }

    std::cout << answer << '\n';
}
