#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, k;
    std::cin >> n >> k;
    std::vector<int> values(n);
    for (int& value : values) {
        long long input;
        std::cin >> input;
        value = static_cast<int>(input);
    }
    std::sort(values.begin(), values.end());
    std::cout << values[k - 1] << '\n';
}
