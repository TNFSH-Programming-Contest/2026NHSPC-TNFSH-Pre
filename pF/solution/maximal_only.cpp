#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;
    std::vector<int> a(2 * n);
    for (int& value : a) std::cin >> value;

    long long answer = 0;
    for (int center = 0; center < 2 * n; ++center) {
        for (int even = 0;
             even < 1 + (center + 1 < 2 * n &&
                         a[center] == a[center + 1]);
             ++even) {
            int left = center;
            int right = center + even;
            int maximum = a[left];
            while (left > 0 && right + 1 < 2 * n &&
                   a[left - 1] == a[right + 1]) {
                --left;
                ++right;
                maximum = std::max(maximum, a[left]);
            }
            // Wrong: all shorter palindromes at this center are omitted.
            answer += maximum;
        }
    }
    std::cout << answer << '\n';
}
