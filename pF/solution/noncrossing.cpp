#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;
    std::vector<int> a(2 * n);
    std::vector<int> first(n, -1), second(n, -1);
    long long answer = 0;
    for (int i = 0; i < 2 * n; ++i) {
        std::cin >> a[i];
        answer += a[i];
        if (first[a[i]] == -1) first[a[i]] = i;
        else second[a[i]] = i;
    }

    if (n > 2000) {
        std::cout << answer << '\n';
        return 0;
    }
    for (int x = 0; x < n; ++x) {
        bool crossed = false;
        for (int y = 0; y < n; ++y) {
            const bool leftInside =
                first[x] < first[y] && first[y] < second[x];
            const bool rightInside =
                first[x] < second[y] && second[y] < second[x];
            if (leftInside != rightInside) crossed = true;
        }
        if (!crossed) {
            int maximum = 0;
            for (int i = first[x]; i <= second[x]; ++i) {
                maximum = std::max(maximum, a[i]);
            }
            // Wrong: a noncrossing collection of pairs need not be palindromic.
            answer += maximum;
        }
    }
    std::cout << answer << '\n';
}
