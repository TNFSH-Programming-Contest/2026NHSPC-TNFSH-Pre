#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    int n, k;
    std::cin >> n >> k;
    std::vector<long long> values(n);
    for (long long& value : values) std::cin >> value;
    std::sort(values.begin(), values.end());

    const int wrongIndex = std::min(k, n - 1);
    std::cout << values[wrongIndex] << '\n';
}
