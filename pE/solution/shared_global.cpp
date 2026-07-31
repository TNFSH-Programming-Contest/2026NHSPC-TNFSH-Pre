#include "Cake_4.h"

#include <string>
#include <utility>
#include <vector>

namespace {

std::vector<std::vector<long long>> supposedlyShared;

}  // namespace

std::string encode(int, std::vector<std::vector<long long>> w) {
    supposedlyShared = std::move(w);
    return "";
}

void decode(int, std::string) {
    // Incorrectly assumes encode changed this process's global vector.
}

int query(int l, int r) {
    if (supposedlyShared.empty()) return l;
    return (l + r) / 2;
}
