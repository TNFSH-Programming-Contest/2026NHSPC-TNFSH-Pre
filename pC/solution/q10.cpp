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
    if (q > 10) return 0;

    constexpr int dr[4] = {-1, 0, 1, 0};
    constexpr int dc[4] = {0, 1, 0, -1};

    while (q--) {
        int entry;
        std::cin >> entry;
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
            column = 2 * m + n - entry;
            direction = 0;
        } else {
            row = 2 * (m + n) - entry;
            column = 0;
            direction = 1;
        }

        while (0 <= row && row < n && 0 <= column && column < m) {
            if (grid[row][column] == '/') direction ^= 1;
            if (grid[row][column] == '\\') direction ^= 3;
            row += dr[direction];
            column += dc[direction];
        }

        if (row < 0) std::cout << column + 1 << '\n';
        else if (column >= m) std::cout << m + row + 1 << '\n';
        else if (row >= n) std::cout << 2 * m + n - column << '\n';
        else std::cout << 2 * (m + n) - row << '\n';
    }
}

