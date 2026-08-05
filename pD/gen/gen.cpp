#include "testlib.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

int argumentCount;
char** arguments;

template <typename T>
T opt(int index) {
    ensuref(index < argumentCount, "missing generator argument %d", index);
    std::istringstream input(arguments[index]);
    T value;
    input >> value;
    ensuref(input && input.eof(), "invalid generator argument %d: %s",
            index, arguments[index]);
    return value;
}

}  // namespace

int main(int argc, char* argv[]) {
    registerGen(argc, argv, 1);
    argumentCount = argc;
    arguments = argv;

    const int n = opt<int>(1);
    const int m = opt<int>(2);
    const int q = opt<int>(3);
    const std::string gridMode = opt<std::string>(4);
    const std::string queryMode = opt<std::string>(5);

    ensuref(1 <= n && n <= 2000, "N must be in [1, 2000]");
    ensuref(1 <= m && m <= 2000, "M must be in [1, 2000]");
    ensuref(1 <= q && q <= 200000, "Q must be in [1, 200000]");

    int nextArgument = 6;
    std::vector<std::string> grid(n, std::string(m, '.'));
    if (gridMode == "dots") {
        // Already initialized.
    } else if (gridMode == "slash") {
        for (std::string& row : grid) row.assign(m, '/');
    } else if (gridMode == "backslash") {
        for (std::string& row : grid) row.assign(m, '\\');
    } else if (gridMode == "checker") {
        for (int row = 0; row < n; ++row) {
            for (int column = 0; column < m; ++column) {
                grid[row][column] = (row + column) % 2 == 0 ? '/' : '\\';
            }
        }
    } else if (gridMode == "random") {
        const int dotWeight = opt<int>(nextArgument++);
        const int slashWeight = opt<int>(nextArgument++);
        const int backslashWeight = opt<int>(nextArgument++);
        ensuref(dotWeight >= 0 && slashWeight >= 0 && backslashWeight >= 0,
                "grid weights must be non-negative");
        const int totalWeight = dotWeight + slashWeight + backslashWeight;
        ensuref(totalWeight > 0, "at least one grid weight must be positive");

        for (std::string& row : grid) {
            for (char& cell : row) {
                const int choice = rnd.next(totalWeight);
                if (choice < dotWeight) {
                    cell = '.';
                } else if (choice < dotWeight + slashWeight) {
                    cell = '/';
                } else {
                    cell = '\\';
                }
            }
        }
    } else if (gridMode == "blocks") {
        const int blockSize = opt<int>(nextArgument++);
        ensuref(blockSize >= 1, "block size must be positive");
        constexpr char cells[3] = {'.', '/', '\\'};
        for (int row = 0; row < n; ++row) {
            for (int column = 0; column < m; ++column) {
                grid[row][column] =
                    cells[((row / blockSize) + (column / blockSize)) % 3];
            }
        }
    } else if (gridMode == "double_snake") {
        ensuref(n >= 5 && m >= 5 && n % 2 == 1 && m % 2 == 1,
                "double_snake requires odd N, M >= 5");

        // One path snakes horizontally through the inner rows, while a
        // second one snakes vertically through the inner columns. Empty
        // cells let the paths cross. Replacing their near-final crossing
        // by a slash splices the two long prefixes into one boundary path
        // whose length approaches 2*N*M.
        for (int transition = 0; transition < n - 3; ++transition) {
            const int row = 1 + transition;
            if (transition % 2 == 0) {
                grid[row][m - 1] = '\\';
                grid[row + 1][m - 1] = '/';
            } else {
                grid[row][0] = '/';
                grid[row + 1][0] = '\\';
            }
        }
        for (int transition = 0; transition < m - 3; ++transition) {
            const int column = 1 + transition;
            if (transition % 2 == 0) {
                grid[n - 1][column] = '\\';
                grid[n - 1][column + 1] = '/';
            } else {
                grid[0][column] = '/';
                grid[0][column + 1] = '\\';
            }
        }
        grid[n - 2][m - 2] = '/';
    } else {
        quitf(_fail, "unknown grid mode: %s", gridMode.c_str());
    }

    const int perimeter = 2 * (n + m);
    std::vector<int> queries(q);
    if (queryMode == "cycle") {
        for (int i = 0; i < q; ++i) queries[i] = i % perimeter + 1;
    } else if (queryMode == "reverse") {
        for (int i = 0; i < q; ++i) queries[i] = perimeter - i % perimeter;
    } else if (queryMode == "random") {
        for (int& entry : queries) entry = rnd.next(1, perimeter);
    } else if (queryMode == "same") {
        const int entry = opt<int>(nextArgument++);
        ensuref(1 <= entry && entry <= perimeter, "query entry is out of range");
        for (int& value : queries) value = entry;
    } else if (queryMode == "corners") {
        const std::vector<int> candidates = {
            1, m, m + 1, m + n, m + n + 1, 2 * m + n,
            2 * m + n + 1, perimeter
        };
        for (int i = 0; i < q; ++i) {
            queries[i] = candidates[i % static_cast<int>(candidates.size())];
        }
    } else {
        quitf(_fail, "unknown query mode: %s", queryMode.c_str());
    }

    std::cout << n << ' ' << m << ' ' << q << '\n';
    for (const std::string& row : grid) std::cout << row << '\n';
    for (int entry : queries) std::cout << entry << '\n';
}
