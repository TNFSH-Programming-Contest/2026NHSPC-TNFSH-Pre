#include <iostream>
#include <vector>

int main() {
    int n, k;
    std::cin >> n >> k;
    std::vector<long long> values(n);
    for (long long& value : values) std::cin >> value;
    std::cout << values[k - 1] << '\n';
}
