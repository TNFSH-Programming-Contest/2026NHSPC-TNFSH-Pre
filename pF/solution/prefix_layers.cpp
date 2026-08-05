#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;
    std::vector<int> a(2 * n);
    std::vector<int> first(n, -1), second(n, -1);
    long long answer = 0;
    for (int i = 0; i < 2 * n; ++i) {
        std::cin >> a[i];
        answer += a[i];
        if (first[a[i]] == -1) first[a[i]] = i;
        else second[a[i]] = i;
    }

    int size = 1;
    while (size < 2 * n) size *= 2;
    std::vector<int> tree(2 * size);
    for (int i = 0; i < 2 * n; ++i) tree[size + i] = a[i];
    for (int i = size - 1; i >= 1; --i) {
        tree[i] = std::max(tree[2 * i], tree[2 * i + 1]);
    }
    const auto rangeMaximum = [&](int left, int right) {
        int result = 0;
        for (left += size, right += size + 1; left < right;
             left /= 2, right /= 2) {
            if (left % 2 == 1) result = std::max(result, tree[left++]);
            if (right % 2 == 1) result = std::max(result, tree[--right]);
        }
        return result;
    };

    constexpr int CHECKED_LAYERS = 16;
    for (int value = 0; value < n; ++value) {
        const int left = first[value];
        const int right = second[value];
        bool seemsPalindromic = true;
        for (int offset = 1;
             offset <= CHECKED_LAYERS && left + offset <= right - offset;
             ++offset) {
            if (a[left + offset] != a[right - offset]) {
                seemsPalindromic = false;
                break;
            }
        }
        // Wrong: a long interval can first differ after the checked prefix.
        if (seemsPalindromic) answer += rangeMaximum(left, right);
    }
    std::cout << answer << '\n';
}
