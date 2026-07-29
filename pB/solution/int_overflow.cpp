#include <algorithm>
#include <climits>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    std::vector<int> a(n), prefix(n + 1);
    for (int& value : a) std::cin >> value;
    std::sort(a.begin(), a.end());
    if (a.front() == a.back()) {
        std::cout << 0 << '\n';
        return 0;
    }
    for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + a[i];

    int answer = INT_MAX;
    for (int i = 0; i < n; ++i) {
        int left = (i + 1) * a[i] - prefix[i + 1];
        int right = (n - i - 1) * (43200 + a[i]) - (prefix[n] - prefix[i + 1]);
        answer = std::min(answer, left + right);
    }
    std::cout << answer << '\n';
}
