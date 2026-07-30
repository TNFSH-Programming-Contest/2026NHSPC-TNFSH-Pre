#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;

    if (n > 1000) {
        std::cout << 0 << '\n';
        return 0;
    }

    std::vector<int> exponents(n + 1, 0);
    for (int value = 2; value <= n; ++value) {
        int remaining = value;
        for (int prime = 2; 1LL * prime * prime <= remaining; ++prime) {
            while (remaining % prime == 0) {
                ++exponents[prime];
                remaining /= prime;
            }
        }
        if (remaining > 1) ++exponents[remaining];
    }

    long long answer = 1;
    for (int exponent : exponents) {
        answer = answer * (exponent + 1) % 1145141;
    }

    std::cout << answer << '\n';
}
