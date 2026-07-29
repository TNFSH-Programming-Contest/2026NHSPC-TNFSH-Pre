#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    std::vector<long long> times(n);
    for (long long& time : times) std::cin >> time;
    std::sort(times.begin(), times.end());

    const long long target = times[n / 2];
    long long answer = 0;
    for (long long time : times) answer += std::llabs(target - time);
    std::cout << answer << '\n';
}
