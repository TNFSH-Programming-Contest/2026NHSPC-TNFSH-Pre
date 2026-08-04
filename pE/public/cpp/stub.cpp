#include "Cake_4.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

int main() {
    int n, q;
    if (!(std::cin >> n >> q)) return 0;
    std::vector<std::vector<long long>> w(
        n, std::vector<long long>(n));
    for (int l = 0; l < n; ++l) {
        for (int r = l; r < n; ++r) std::cin >> w[l][r];
    }

    const std::string encoded = encode(n, std::move(w));
    if (encoded.size() > 4000000) {
        std::cerr << "encoded string is too long\n";
        return 1;
    }
    for (char c : encoded) {
        if (c != '0' && c != '1') {
            std::cerr << "encoded string is not binary\n";
            return 1;
        }
    }

    decode(n, encoded);
    for (int i = 0; i < q; ++i) {
        int l, r;
        std::cin >> l >> r;
        std::cout << query(l, r) << '\n';
    }
}
