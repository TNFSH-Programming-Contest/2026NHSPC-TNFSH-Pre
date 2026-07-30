#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;

    std::vector<int> primes;
    std::vector<bool> composite(n + 1, false);
    for (int value = 2; value <= n; ++value) {
        if (!composite[value]) primes.push_back(value);
        for (int prime : primes) {
            if (1LL * value * prime > n) break;
            composite[value * prime] = true;
            if (value % prime == 0) break;
        }
    }

    constexpr long long WRONG_MOD = 1000000007;
    long long answer = 1;
    for (int prime : primes) {
        long long exponent = 0;
        for (long long power = prime; power <= n; power *= prime) {
            exponent += n / power;
            if (power > n / prime) break;
        }
        answer = answer * (exponent + 1) % WRONG_MOD;
    }

    std::cout << answer << '\n';
}
