#include <iostream>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;

    if (n > 15) {
        std::cout << 0 << '\n';
        return 0;
    }

    unsigned long long factorial = 1;
    for (int value = 2; value <= n; ++value) factorial *= value;

    long long divisors = 0;
    for (unsigned long long divisor = 1; divisor * divisor <= factorial; ++divisor) {
        if (factorial % divisor != 0) continue;
        divisors += 1;
        if (divisor * divisor != factorial) divisors += 1;
    }

    std::cout << divisors % 1145141 << '\n';
}
