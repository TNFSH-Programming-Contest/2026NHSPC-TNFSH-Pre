#include "testlib.h"

#include <iostream>
#include <sstream>
#include <string>

namespace {

template <typename T>
T parseArgument(const char* text, const char* name) {
    std::istringstream input(text);
    T value;
    input >> value;
    ensuref(input && input.eof(), "invalid %s: %s", name, text);
    return value;
}

}  // namespace

int main(int argc, char* argv[]) {
    registerGen(argc, argv, 1);
    ensuref(argc >= 2, "usage: gen n");

    const int n = parseArgument<int>(argv[1], "n");
    ensuref(1 <= n && n <= 1000000000, "n must be in [1, 1000000000]");

    std::cout << n << '\n';
}
