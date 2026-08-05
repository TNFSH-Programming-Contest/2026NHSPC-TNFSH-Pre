#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;
    std::vector<int> a(2 * n);
    std::vector<int> first(n, -1);
    long long answer = 0;
    for (int i = 0; i < 2 * n; ++i) {
        std::cin >> a[i];
        answer += a[i];
        if (first[a[i]] == -1) first[a[i]] = i;
    }

    for (int value = 0; value < n; ++value) {
        int maximum = 0;
        int right = first[value] + 1;
        while (right < 2 * n && a[right] != value) ++right;
        for (int i = first[value]; i <= right; ++i) {
            maximum = std::max(maximum, a[i]);
        }
        // Wrong: equal endpoints do not make the whole interval a palindrome.
        answer += maximum;
    }
    std::cout << answer << '\n';
}
