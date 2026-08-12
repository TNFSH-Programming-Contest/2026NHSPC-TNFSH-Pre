#include "Cake_4.h"

#include <string>
#include <utility>
#include <vector>

namespace {
using Matrix = std::array<std::array<long long, 2000>, 2000>;

Matrix supposedlyShared;

}  // namespace

std::string encode(int, const Matrix& w) {
    supposedlyShared = std::move(w);
    return "";
}

void decode(int, const std::string&) {
    // Incorrectly assumes encode changed this process's global vector.
}

int query(int l, int r) {
    if (supposedlyShared.empty()) return l;
    return (l + r) / 2;
}
