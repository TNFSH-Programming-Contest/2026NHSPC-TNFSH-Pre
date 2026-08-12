#include "Cake_4.h"

#include <string>

using Matrix = std::array<std::array<long long, 2000>, 2000>;
std::string encode(int, const Matrix&) {
    return "0";
}

void decode(int, const std::string&) {}

int query(int, int r) {
    return r;
}
