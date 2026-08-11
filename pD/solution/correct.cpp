#include <iostream>
#include <vector>

namespace {

constexpr long long MOD = 1145141;

}  // namespace

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

    long long answer = 1;
    for (int prime : primes) {
        long long exponent = 0;
        long long power = prime;
        while (power <= n) {
            exponent += n / power;
            if (power > n / prime) break;
            power *= prime;
        }
        answer = answer * (exponent + 1) % MOD;
    }

    std::cout << answer << '\n';
}
