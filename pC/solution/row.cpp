#include <iostream>
#include <string>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, m, q;
    std::cin >> n >> m >> q;

    std::vector<int> mirrorColumn(n, -1);
    std::vector<char> mirrorType(n, '.');
    for (int row = 0; row < n; ++row) {
        std::string cells;
        std::cin >> cells;
        for (int column = 0; column < m; ++column) {
            if (cells[column] == '.') continue;
            if (mirrorColumn[row] != -1) return 0;
            mirrorColumn[row] = column;
            mirrorType[row] = cells[column];
        }
    }

    std::vector<int> first(m, -1), last(m, -1);
    std::vector<int> previous(n, -1), next(n, -1);
    for (int row = 0; row < n; ++row) {
        const int column = mirrorColumn[row];
        if (column == -1) continue;
        if (first[column] == -1) first[column] = row;
        previous[row] = last[column];
        if (last[column] != -1) next[last[column]] = row;
        last[column] = row;
    }

    const int perimeter = 2 * (n + m);
    auto leaveAfterVerticalHit = [&](int row, bool movingDown) {
        if ((movingDown && mirrorType[row] == '/') ||
            (!movingDown && mirrorType[row] == '\\')) {
            return perimeter - row;
        }
        return m + row + 1;
    };

    while (q--) {
        int entry;
        std::cin >> entry;

        if (entry <= m) {
            const int column = entry - 1;
            const int hit = first[column];
            if (hit == -1) {
                std::cout << 2 * m + n - column << '\n';
            } else {
                std::cout << leaveAfterVerticalHit(hit, true) << '\n';
            }
            continue;
        }

        if (entry > m + n && entry <= 2 * m + n) {
            const int column = 2 * m + n - entry;
            const int hit = last[column];
            if (hit == -1) {
                std::cout << column + 1 << '\n';
            } else {
                std::cout << leaveAfterVerticalHit(hit, false) << '\n';
            }
            continue;
        }

        const bool fromLeft = entry > 2 * m + n;
        const int row = fromLeft ? perimeter - entry : entry - m - 1;
        const int column = mirrorColumn[row];
        if (column == -1) {
            std::cout << (fromLeft ? m + row + 1 : perimeter - row) << '\n';
            continue;
        }

        const bool movingDown =
            (fromLeft && mirrorType[row] == '\\') ||
            (!fromLeft && mirrorType[row] == '/');
        const int secondHit = movingDown ? next[row] : previous[row];
        if (secondHit != -1) {
            std::cout << leaveAfterVerticalHit(secondHit, movingDown) << '\n';
        } else if (movingDown) {
            std::cout << 2 * m + n - column << '\n';
        } else {
            std::cout << column + 1 << '\n';
        }
    }
}

