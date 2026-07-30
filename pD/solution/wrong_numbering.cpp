#include <iostream>
#include <string>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, m, q;
    std::cin >> n >> m >> q;
    std::vector<std::string> grid(n);
    for (std::string& row : grid) std::cin >> row;

    const int perimeter = 2 * (n + m);
    std::vector<int> answer(perimeter + 1, 0);
    constexpr int dr[4] = {-1, 0, 1, 0};
    constexpr int dc[4] = {0, 1, 0, -1};

    auto trace = [&](int entry) {
        int row, column, direction;
        if (entry <= m) {
            row = 0;
            column = entry - 1;
            direction = 2;
        } else if (entry <= m + n) {
            row = entry - m - 1;
            column = m - 1;
            direction = 3;
        } else if (entry <= 2 * m + n) {
            row = n - 1;
            column = entry - (m + n + 1);
            direction = 0;
        } else {
            row = entry - (2 * m + n + 1);
            column = 0;
            direction = 1;
        }

        while (0 <= row && row < n && 0 <= column && column < m) {
            if (grid[row][column] == '/') {
                direction ^= 1;
            } else if (grid[row][column] == '\\') {
                direction ^= 3;
            }
            row += dr[direction];
            column += dc[direction];
        }

        if (row < 0) return column + 1;
        if (column >= m) return m + row + 1;
        if (row >= n) return m + n + column + 1;
        return 2 * m + n + row + 1;
    };

    while (q--) {
        int entry;
        std::cin >> entry;
        if (answer[entry] == 0) answer[entry] = trace(entry);
        std::cout << answer[entry] << '\n';
    }
}
