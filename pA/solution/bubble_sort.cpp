#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, k;
    std::cin >> n >> k;

    std::vector<long long> values(n);
    for (long long& value : values) std::cin >> value;

    for (int last = n - 1; last > 0; --last) {
        for (int i = 0; i < last; ++i) {
            if (values[i] > values[i + 1]) {
                std::swap(values[i], values[i + 1]);
            }
        }
    }

    std::cout << values[k - 1] << '\n';
}
