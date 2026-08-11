#include <iostream>

int main(void) {
    int n, m, q, pos;
    char ch;
    std::cin >> n >> m >> q;
    if (n * m != 1) return 0;
    std::cin >> ch;

    while (q--) {
        std::cin >> pos;
        switch (ch) {
            case '.':
                switch (pos) {
                    case 1: std::cout << "3\n"; break;
                    case 2: std::cout << "4\n"; break;
                    case 3: std::cout << "1\n"; break;
                    case 4: std::cout << "2\n"; break;
                    default: break;
                }
                break;
            case '/':
                switch (pos) {
                    case 1: std::cout << "4\n"; break;
                    case 2: std::cout << "3\n"; break;
                    case 3: std::cout << "2\n"; break;
                    case 4: std::cout << "1\n"; break;
                    default: break;
                }
                break;
            case '\\':
                switch (pos) {
                    case 1: std::cout << "2\n"; break;
                    case 2: std::cout << "1\n"; break;
                    case 3: std::cout << "4\n"; break;
                    case 4: std::cout << "3\n"; break;
                    default: break;
                }
                break;
            default:
                break;
        }
    }
}
