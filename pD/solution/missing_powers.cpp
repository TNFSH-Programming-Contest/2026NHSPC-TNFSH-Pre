#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;

    std::vector<bool> composite(n + 1, false);
    long long answer = 1;
    for (int prime = 2; prime <= n; ++prime) {
        if (composite[prime]) continue;

        const int exponent = n / prime;
        answer = answer * (exponent + 1) % 1145141;

        if (1LL * prime * prime <= n) {
            for (int multiple = prime * prime; multiple <= n; multiple += prime) {
                composite[multiple] = true;
            }
        }
    }

    std::cout << answer << '\n';
}
