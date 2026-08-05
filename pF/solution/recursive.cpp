#include <algorithm>
#include <iostream>
#include <vector>

namespace {

std::vector<int> values;
long long answer;
volatile int recursionGuard;

__attribute__((noinline)) void expand(int left, int right, int maximum) {
    answer += maximum;
    if (left > 0 && right + 1 < static_cast<int>(values.size()) &&
        values[left - 1] == values[right + 1]) {
        expand(left - 1, right + 1,
               std::max(maximum, values[left - 1]));
        recursionGuard ^= left;
    }
}

}  // namespace

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;
    values.resize(2 * n);
    for (int& value : values) std::cin >> value;

    for (int center = 0; center < 2 * n; ++center) {
        expand(center, center, values[center]);
        if (center + 1 < 2 * n && values[center] == values[center + 1]) {
            expand(center, center + 1, values[center]);
        }
    }
    std::cout << answer << '\n';
}
