#include "Cake_4.h"

#include <string>
#include <vector>

using Matrix = std::array<std::array<long long, 2000>, 2000>;
std::string encode(int, const Matrix&) {
    return "";
}

void decode(int, const std::string&) {}

int query(int l, int r) {
    return (l + r) / 2;
}
