#include "Cake_4.h"

#include <array>
#include <vector>
#include <csignal>
#include <fstream>
#include <limits>
#include <string>
#include <cstring>

namespace {
constexpr std::size_t BIT_LIMIT = 4000000;
std::array<std::array<long long, 2000>, 2000> w;

class FastInput {
    static constexpr size_t BUF_SIZE = 1 << 20; // 1 MB

    FILE* fp;
    char buf[BUF_SIZE];
    size_t pos = 0, len = 0;

    inline char getChar() {
        if (pos >= len) {
            len = std::fread(buf, 1, BUF_SIZE, fp);
            pos = 0;
            if (len == 0) return '\0';
        }
        return buf[pos++];
    }

public:
    explicit FastInput(const char* filename) {
        fp = std::fopen(filename, "rb");
    }

    ~FastInput() {
        if (fp) std::fclose(fp);
    }

    bool good() const {
        return fp != nullptr;
    }

    bool readInt(int& out) {
        char c;
        do {
            c = getChar();
            if (!c) return false;
        } while (c <= ' ');

        int sign = 1;
        if (c == '-') {
            sign = -1;
            c = getChar();
        }

        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = getChar();
        }

        out = x * sign;
        return true;
    }

    bool readLongLong(long long& out) {
        char c;
        do {
            c = getChar();
            if (!c) return false;
        } while (c <= ' ');

        long long sign = 1;
        if (c == '-') {
            sign = -1;
            c = getChar();
        }

        long long x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10LL + (c - '0');
            c = getChar();
        }

        out = x * sign;
        return true;
    }

    bool readSizeT(size_t& out) {
        char c;
        do {
            c = getChar();
            if (!c) return false;
        } while (c <= ' ');

        size_t x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = getChar();
        }

        out = x;
        return true;
    }

    bool readRawString(size_t n, std::string& out) {
        out.resize(n);

        size_t copied = 0;
        while (copied < n) {
            if (pos >= len) {
                len = std::fread(buf, 1, BUF_SIZE, fp);
                pos = 0;
                if (len == 0) return false;
            }

            size_t available = len - pos;
            size_t take = std::min(available, n - copied);

            std::memcpy((void*)(out.data() + copied), buf + pos, take);

            pos += take;
            copied += take;
        }

        return true;
    }
};

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
    // std::ifstream fromManager(argv[1]);
    FastInput fromManager(argv[1]);
    if (!toManager || !fromManager.good()) return 0;

    int processIndex;
    try {
        processIndex = std::stoi(argv[3]);
    } catch (...) {
        return 0;
    }

    if (processIndex == 0) {
        int n;
        // if (!(fromManager >> n)) return 0;
        if (!(fromManager.readInt(n))) return 0;
        for (int l = 0; l < n; ++l) {
            for (int r = l; r < n; ++r) {
                // if (!(fromManager >> w[l][r])) return 0;
                if (!(fromManager.readLongLong(w[l][r]))) return 0;
            }
        }

        const std::string encoded = encode(n, w);
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
        // if (!(fromManager >> n >> encodedLength)) return 0;
        if (!fromManager.readInt(n) || !fromManager.readSizeT(encodedLength))
            return 0;
        if (encodedLength > BIT_LIMIT) return 0;

        std::string encoded;
        // if (!readRawString(fromManager, encodedLength, encoded)) return 0;
        if (!fromManager.readRawString(encodedLength, encoded))
            return 0;

        decode(n, encoded);

        int q;
        // if (!(fromManager >> q)) return 0;
        if (!fromManager.readInt(q)) return 0;
        std::vector<int> answers;
        answers.reserve(q);
        for (int i = 0; i < q; ++i) {
            int l, r;
            // if (!(fromManager >> l >> r)) return 0;
            if (!(fromManager.readInt(l) && fromManager.readInt(r))) return 0;
            answers.push_back(query(l, r));
        }

        for (int answer : answers) toManager << answer << '\n';
        toManager.flush();
    }
    return 0;
}
