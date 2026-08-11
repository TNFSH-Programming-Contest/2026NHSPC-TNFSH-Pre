#include <iostream>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;

    long long answer = 1;
    int remaining = n;
    for (int prime = 2; 1LL * prime * prime <= remaining; ++prime) {
        int exponent = 0;
        while (remaining % prime == 0) {
            remaining /= prime;
            ++exponent;
        }
        answer = answer * (exponent + 1) % 1145141;
    }
    if (remaining > 1) answer = answer * 2 % 1145141;

    std::cout << answer << '\n';
}
