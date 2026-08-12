#include "Cake_4.h"

#include <iostream>
#include <string>

void wrong(const std::string s) {
    std::cerr << "Wrong Answer: " << s << '\n';
    exit(0);
}

std::array<std::array<long long, 2000>, 2000> w;

int main() {
    const int B = 4e6;
    int n, q;
    std::cin >> n >> q;
    for (int l = 0; l < n; l++) {
        for (int r = l; r < n; r++) std::cin >> w[l][r];
    }

    const std::string encoded = encode(n, w);
    if (encoded.size() > B) {
        wrong("Length limit exceeded");
    }
    for (char c : encoded) {
        if (c != '0' && c != '1') {
            wrong("Invaild returned character");
        }
    }

    std::cout << encoded << '\n';

    decode(n, encoded);
    for (int i = 0; i < q; i++) {
        int l, r;
        std::cin >> l >> r;
        std::cout << i << ' ' << "query(" << l << ',' << r << ") " << query(l, r) << '\n';
    }
}
