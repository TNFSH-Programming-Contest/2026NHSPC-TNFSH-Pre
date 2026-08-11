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
    bool hasSlash = false, hasBackslash = false;
    for (const std::string& row : grid) {
        for (char cell : row) {
            hasSlash |= cell == '/';
            hasBackslash |= cell == '\\';
        }
    }
    if (hasSlash && hasBackslash) return 0;

    std::vector<int> queries(q);
    for (int& entry : queries) std::cin >> entry;

    const int perimeter = 2 * (n + m);
    std::vector<int> answer(perimeter + 1);
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
            column = 2 * m + n - entry;
            direction = 0;
        } else {
            row = perimeter - entry;
            column = 0;
            direction = 1;
        }

        while (0 <= row && row < n && 0 <= column && column < m) {
            if (grid[row][column] == '/') direction ^= 1;
            if (grid[row][column] == '\\') direction ^= 3;
            row += dr[direction];
            column += dc[direction];
        }

        if (row < 0) return column + 1;
        if (column >= m) return m + row + 1;
        if (row >= n) return 2 * m + n - column;
        return perimeter - row;
    };

    for (int entry : queries) {
        if (answer[entry] == 0) {
            const int exit = trace(entry);
            answer[entry] = exit;
            answer[exit] = entry;
        }
        std::cout << answer[entry] << '\n';
    }
}

