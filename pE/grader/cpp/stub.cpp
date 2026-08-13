#include "Cake_4.h"

#include <array>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <string>
#include <type_traits>
#include <vector>

namespace {

constexpr std::size_t BIT_LIMIT = 4000000;
constexpr int MATRIX_SIZE = 2000;
using Matrix =
    std::array<std::array<long long, MATRIX_SIZE>, MATRIX_SIZE>;

Matrix w;

static_assert(sizeof(long long) == sizeof(std::int64_t),
              "stub requires 64-bit long long");
static_assert(std::is_standard_layout<Matrix>::value,
              "Matrix must have standard layout");
static_assert(sizeof(Matrix) ==
                  static_cast<std::size_t>(MATRIX_SIZE) * MATRIX_SIZE *
                      sizeof(long long),
              "Matrix rows must be contiguous and contain no padding");

bool readBytes(FILE* input, void* destination, std::size_t size) {
    char* output = static_cast<char*>(destination);
    while (size != 0) {
        const std::size_t received = std::fread(output, 1, size, input);
        if (received == 0) return false;
        output += received;
        size -= received;
    }
    return true;
}

bool writeBytes(FILE* output, const void* source, std::size_t size) {
    const char* input = static_cast<const char*>(source);
    while (size != 0) {
        const std::size_t sent = std::fwrite(input, 1, size, output);
        if (sent == 0) return false;
        input += sent;
        size -= sent;
    }
    return true;
}

template <typename T>
bool readBinary(FILE* input, T& value) {
    return readBytes(input, &value, sizeof(value));
}

template <typename T>
bool writeBinary(FILE* output, const T& value) {
    return writeBytes(output, &value, sizeof(value));
}

}  // namespace

int main(int argc, char* argv[]) {
    std::signal(SIGPIPE, SIG_IGN);
    if (argc < 4) return 0;

    /*
     * These files are named pipes.  Keep the writer-first open order paired
     * with manager.cpp's reader-first order so opening the FIFOs cannot
     * deadlock.
     */
    FILE* toManager = std::fopen(argv[2], "wb");
    FILE* fromManager = std::fopen(argv[1], "rb");
    if (toManager == nullptr || fromManager == nullptr) return 0;

    int processIndex;
    try {
        processIndex = std::stoi(argv[3]);
    } catch (...) {
        return 0;
    }

    if (processIndex == 0) {
        std::uint32_t binaryN;
        if (!readBinary(fromManager, binaryN) ||
            binaryN < 2 || binaryN > MATRIX_SIZE) {
            return 0;
        }
        const int n = static_cast<int>(binaryN);
        const std::size_t matrixBytes =
            static_cast<std::size_t>(n) * sizeof(w[0]);

        /*
         * std::array is contiguous.  fread therefore materializes the first n
         * rows of the 2000 x 2000 matrix directly in w, with no per-cell
         * parsing or copying.
         */
        if (!readBytes(fromManager, w.data(), matrixBytes)) return 0;
        std::fclose(fromManager);

        const std::string encoded = encode(n, w);
        const std::uint64_t encodedLength = encoded.size();
        if (!writeBinary(toManager, encodedLength)) return 0;
        if (encodedLength <= BIT_LIMIT && !encoded.empty() &&
            !writeBytes(toManager, encoded.data(), encoded.size())) {
            return 0;
        }
        std::fflush(toManager);
        std::fclose(toManager);
        return 0;
    }

    if (processIndex == 1) {
        std::uint32_t binaryN;
        std::uint64_t encodedLength;
        if (!readBinary(fromManager, binaryN) ||
            binaryN < 2 || binaryN > MATRIX_SIZE ||
            !readBinary(fromManager, encodedLength) ||
            encodedLength > BIT_LIMIT) {
            return 0;
        }
        const int n = static_cast<int>(binaryN);

        std::string encoded(static_cast<std::size_t>(encodedLength), '\0');
        if (!encoded.empty() &&
            !readBytes(fromManager, &encoded[0], encoded.size())) {
            return 0;
        }
        decode(n, encoded);

        std::uint32_t binaryQ;
        if (!readBinary(fromManager, binaryQ) || binaryQ > 100000) return 0;
        const int q = static_cast<int>(binaryQ);
        std::vector<std::int32_t> answers(q);
        for (int i = 0; i < q; ++i) {
            std::int32_t l;
            std::int32_t r;
            if (!readBinary(fromManager, l) ||
                !readBinary(fromManager, r)) {
                return 0;
            }
            answers[i] = query(l, r);
        }
        std::fclose(fromManager);

        if (!writeBytes(toManager, answers.data(),
                        answers.size() * sizeof(answers[0]))) {
            return 0;
        }
        std::fflush(toManager);
        std::fclose(toManager);
    }
    return 0;
}
