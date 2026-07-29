#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    std::vector<int> times(n);
    for (int& time : times) std::cin >> time;
    if (std::all_of(times.begin(), times.end(),
                    [&](int time) { return time == times.front(); })) {
        std::cout << 0 << '\n';
        return 0;
    }


    long long answer = std::numeric_limits<long long>::max();
    for (int target : times) {
        long long cost = 0;
        for (int time : times) {
            int difference = std::abs(target - time);
            cost += std::min(difference, 43200 - difference);
        }
        answer = std::min(answer, cost);
    }
    std::cout << answer << '\n';
}
