#include <algorithm>
#include <cstddef>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;
    const int length = 2 * n;
    std::vector<int> a(length);
    for (int& value : a) std::cin >> value;

    std::vector<unsigned char> palindrome(
        static_cast<std::size_t>(length) * length);
    long long answer = 0;
    for (int left = length - 1; left >= 0; --left) {
        int maximum = 0;
        for (int right = left; right < length; ++right) {
            maximum = std::max(maximum, a[right]);
            const bool inside =
                right - left <= 1 ||
                palindrome[static_cast<std::size_t>(left + 1) * length +
                           right - 1];
            if (a[left] == a[right] && inside) {
                palindrome[static_cast<std::size_t>(left) * length + right] = 1;
                answer += maximum;
            }
        }
    }
    std::cout << answer << '\n';
}
