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
        int left = center;
        int right = center;
        int maximum = a[center];
        while (true) {
            answer += maximum;
            if (left == 0 || right + 1 == 2 * n ||
                a[left - 1] != a[right + 1]) {
                break;
            }
            --left;
            ++right;
            maximum = std::max(maximum, a[left]);
        }
    }
    std::cout << answer << '\n';
}
