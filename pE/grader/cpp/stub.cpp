#include "Cake_4.h"

#include <csignal>
#include <fstream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr std::size_t BIT_LIMIT = 4000000;

bool readRawString(std::ifstream& input, std::size_t length, std::string& value) {
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    value.assign(length, '\0');
    if (length != 0) {
        input.read(&value[0], static_cast<std::streamsize>(length));
        if (input.gcount() != static_cast<std::streamsize>(length)) return false;
    }
    char newline;
    return static_cast<bool>(input.get(newline)) && newline == '\n';
}

}  // namespace

int main(int argc, char* argv[]) {
    std::signal(SIGPIPE, SIG_IGN);
    if (argc < 4) return 0;

    std::ofstream toManager(argv[2]);
    std::ifstream fromManager(argv[1]);
    if (!toManager || !fromManager) return 0;

    int processIndex;
    try {
        processIndex = std::stoi(argv[3]);
    } catch (...) {
        return 0;
    }

    if (processIndex == 0) {
        int n;
        if (!(fromManager >> n)) return 0;
        std::vector<std::vector<long long>> w(
            n, std::vector<long long>(n));
        for (int l = 0; l < n; ++l) {
            for (int r = l; r < n; ++r) {
                if (!(fromManager >> w[l][r])) return 0;
            }
        }

        const std::string encoded = encode(n, std::move(w));
        toManager << encoded.size() << '\n';
        if (encoded.size() <= BIT_LIMIT) {
            toManager.write(encoded.data(),
                            static_cast<std::streamsize>(encoded.size()));
            toManager << '\n';
        }
        toManager.flush();
        return 0;
    }

    if (processIndex == 1) {
        int n;
        std::size_t encodedLength;
        if (!(fromManager >> n >> encodedLength)) return 0;
        if (encodedLength > BIT_LIMIT) return 0;

        std::string encoded;
        if (!readRawString(fromManager, encodedLength, encoded)) return 0;
        decode(n, std::move(encoded));

        int q;
        if (!(fromManager >> q)) return 0;
        std::vector<int> answers;
        answers.reserve(q);
        for (int i = 0; i < q; ++i) {
            int l, r;
            if (!(fromManager >> l >> r)) return 0;
            answers.push_back(query(l, r));
        }

        for (int answer : answers) toManager << answer << '\n';
        toManager.flush();
    }
    return 0;
}
