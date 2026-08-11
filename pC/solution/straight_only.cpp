#include <iostream>
#include <string>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, m, q;
    std::cin >> n >> m >> q;
    std::string row;
    for (int i = 0; i < n; ++i) std::cin >> row;

    const int perimeter = 2 * (n + m);
    while (q--) {
        int entry;
        std::cin >> entry;

        int exit;
        if (entry <= m) {
            exit = 2 * m + n - entry + 1;
        } else if (entry <= m + n) {
            const int tableRow = entry - m;
            exit = perimeter - tableRow + 1;
        } else if (entry <= 2 * m + n) {
            exit = 2 * m + n - entry + 1;
        } else {
            const int tableRow = perimeter - entry + 1;
            exit = m + tableRow;
        }
        std::cout << exit << '\n';
    }
}
