#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    int n, k;
    std::cin >> n >> k;
    std::vector<long long> values(n);
    for (long long& value : values) std::cin >> value;
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());

    const int index = std::min<int>(k - 1, values.size() - 1);
    std::cout << values[index] << '\n';
}
