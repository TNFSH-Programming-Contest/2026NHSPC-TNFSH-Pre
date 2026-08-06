#include "Cake_4.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

void wrong(const std::string s) {
    std::cerr << "Wrong Answer: " << s << '\n';
    exit(0);
}

int main() {
    const int B = 4e6;
    int n, q;
    std::cin >> n >> q;
    std::vector<std::vector<long long>> w(
        n, std::vector<long long>(n));
    for (int l = 0; l < n; l++) {
        for (int r = l; r < n; r++) std::cin >> w[l][r];
    }

    const std::string encoded = encode(n, std::move(w));
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
