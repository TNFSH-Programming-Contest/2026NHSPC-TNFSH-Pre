#include <algorithm>
#include <array>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>

namespace {

constexpr std::size_t BIT_LIMIT = 4000000;
constexpr int MATRIX_SIZE = 2000;
constexpr char TESTCASE_MAGIC[8] = {'C', 'A', 'K', 'E', '4', 'B', 'I', 'N'};
using Matrix =
    std::array<std::array<long long, MATRIX_SIZE>, MATRIX_SIZE>;

struct Query {
    std::int32_t l;
    std::int32_t r;
};

Matrix w;
std::ofstream managerLog;

static_assert(sizeof(long long) == sizeof(std::int64_t),
              "manager requires 64-bit long long");
static_assert(sizeof(Query) == 2 * sizeof(std::int32_t),
              "Query must not contain padding");

[[noreturn]] void finish(double score, const std::string& message) {
    std::cout << score << '\n';
    std::cerr << message << '\n';
    if (managerLog) managerLog << message << '\n';
    std::exit(0);
}

[[noreturn]] void wrong(const std::string& message) {
    finish(0.0, message);
}

bool readBytes(std::istream& input, void* destination, std::size_t size) {
    input.read(static_cast<char*>(destination),
               static_cast<std::streamsize>(size));
    return input.gcount() == static_cast<std::streamsize>(size);
}

bool writeBytes(std::ostream& output, const void* source, std::size_t size) {
    output.write(static_cast<const char*>(source),
                 static_cast<std::streamsize>(size));
    return static_cast<bool>(output);
}

template <typename T>
bool readBinary(std::istream& input, T& value) {
    return readBytes(input, &value, sizeof(value));
}

template <typename T>
bool writeBinary(std::ostream& output, const T& value) {
    return writeBytes(output, &value, sizeof(value));
}

const int A = 1999000;
const int B = BIT_LIMIT;
const double BRAIN_ROT = 0.67;
const int YJSP = 114514;

double y(int x) {
    return (static_cast<double>(B) - static_cast<double>(x)) / (static_cast<double>(B) - static_cast<double>(A));
}

double z(double y) {
    if (y <= 0) return 0;
    if (0 < y && y < 1) return y;
    if (y >= 1) return 1;
}

double f_of_q(double q) {
    double exp = -(q - BRAIN_ROT);
    return 1 / (1 + std::pow(YJSP, exp));
}

double score(int x) {
    if (x <= A) return 1;
    if (x > B) return 0;

    const double half = 1.0/2.0;
    const double f_of_0 = f_of_q(0);
    const double f_of_1 = f_of_q(1);
    const double f_of_z = f_of_q(z(y(x)));
    const double tmp = (f_of_z - f_of_0) / (f_of_1 - f_of_0);
    return half * (1.0 + tmp);
}

}  // namespace

int main(int argc, char* argv[]) {
    std::signal(SIGPIPE, SIG_IGN);
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    if (argc < 5) wrong("Judge Error: manager received too few FIFO arguments");
    if (argc >= 6) managerLog.open(argv[5]);

    std::ifstream fromEncoder(argv[1], std::ios::binary);
    std::ofstream toEncoder(argv[2], std::ios::binary);
    std::ifstream fromDecoder(argv[3], std::ios::binary);
    std::ofstream toDecoder(argv[4], std::ios::binary);
    if (!fromEncoder || !toEncoder || !fromDecoder || !toDecoder) {
        wrong("Judge Error: cannot open communication FIFO");
    }

    char magic[sizeof(TESTCASE_MAGIC)];
    std::uint32_t binarySubtaskLength;
    std::uint32_t binaryN;
    std::uint32_t binaryQ;
    if (!readBytes(std::cin, magic, sizeof(magic)) ||
        std::memcmp(magic, TESTCASE_MAGIC, sizeof(magic)) != 0 ||
        !readBinary(std::cin, binaryN) ||
        !readBinary(std::cin, binaryQ) ||
        !readBinary(std::cin, binarySubtaskLength)) {
        wrong("Judge Error: invalid binary testcase header");
    }
    auto subtask = std::make_unique<char[]>(binarySubtaskLength + 1);
    if (!readBytes(std::cin, subtask.get(), binarySubtaskLength)) {
        wrong("Judge Error: truncated binary subtask string");
    }
    if (binaryN < 2 || binaryN > MATRIX_SIZE ||
        binaryQ < 1 || binaryQ > 100000) {
        wrong("Judge Error: binary testcase dimensions are out of range");
    }
    const int n = static_cast<int>(binaryN);
    const int q = static_cast<int>(binaryQ);

    const std::size_t matrixBytes =
        static_cast<std::size_t>(n) * sizeof(w[0]);
    if (!readBytes(std::cin, w.data(), matrixBytes)) {
        wrong("Judge Error: truncated binary W array");
    }

    std::vector<long long> dp(static_cast<std::size_t>(n) * n);
    if (!readBytes(std::cin, dp.data(), dp.size() * sizeof(dp[0]))) {
        wrong("Judge Error: truncated binary DP array");
    }

    std::vector<Query> queries(q);
    if (!readBytes(std::cin, queries.data(),
                   queries.size() * sizeof(queries[0]))) {
        wrong("Judge Error: truncated binary query list");
    }

    if (!writeBinary(toEncoder, binaryN) ||
        !writeBytes(toEncoder, w.data(), matrixBytes)) {
        wrong("Wrong Answer: encoder stopped before reading input");
    }
    toEncoder.flush();
    if (!toEncoder) wrong("Wrong Answer: encoder stopped before reading input");
    toEncoder.close();

    std::uint64_t encodedLength;
    if (!readBinary(fromEncoder, encodedLength)) {
        wrong("Wrong Answer (1): encoder did not return a string");
    }
    if (encodedLength > BIT_LIMIT) {
        wrong("Wrong Answer (1): encoded string is longer than B");
    }

    std::string encoded(static_cast<std::size_t>(encodedLength), '\0');
    if (!encoded.empty() &&
        !readBytes(fromEncoder, &encoded[0], encoded.size())) {
        wrong("Wrong Answer (1): malformed encoded string");
    }
    if (!std::all_of(encoded.begin(), encoded.end(),
                     [](char c) { return c == '0' || c == '1'; })) {
        wrong("Wrong Answer (1): encoded string is not binary");
    }
    fromEncoder.close();

    const std::uint64_t decoderEncodedLength = encoded.size();
    if (!writeBinary(toDecoder, binaryN) ||
        !writeBinary(toDecoder, decoderEncodedLength) ||
        (!encoded.empty() &&
         !writeBytes(toDecoder, encoded.data(), encoded.size())) ||
        !writeBinary(toDecoder, binaryQ) ||
        !writeBytes(toDecoder, queries.data(),
                    queries.size() * sizeof(queries[0]))) {
        wrong("Wrong Answer: decoder stopped before reading input");
    }
    toDecoder.flush();
    if (!toDecoder) wrong("Wrong Answer: decoder stopped before reading input");
    toDecoder.close();

    std::vector<std::int32_t> answers(q);
    if (!readBytes(fromDecoder, answers.data(),
                   answers.size() * sizeof(answers[0]))) {
        wrong("Wrong Answer: decoder did not answer every query");
    }

    const auto index = [n](int l, int r) {
        return static_cast<std::size_t>(l) * n + r;
    };
    int firstBadType = 0;
    int firstBadQuery = -1;
    int firstBadAnswer = 0;
    for (int i = 0; i < q; ++i) {
        const int answer = answers[i];
        const int l = queries[i].l;
        const int r = queries[i].r;
        if (firstBadType == 0 && !(l <= answer && answer < r)) {
            firstBadType = 2;
            firstBadQuery = i;
            firstBadAnswer = answer;
        } else if (firstBadType == 0) {
            const long long candidate =
                dp[index(l, answer)] + dp[index(answer + 1, r)];
            if (candidate != dp[index(l, r)] - w[l][r]) {
                firstBadType = 3;
                firstBadQuery = i;
                firstBadAnswer = answer;
            }
        }
    }

    if (firstBadType == 2) {
        wrong("Wrong Answer (2): query " + std::to_string(firstBadQuery + 1) +
              " returned illegal cut " + std::to_string(firstBadAnswer));
    }
    if (firstBadType == 3) {
        wrong("Wrong Answer (3): query " + std::to_string(firstBadQuery + 1) +
              " returned a non-optimal cut " +
              std::to_string(firstBadAnswer));
    }
    if (memcmp(subtask.get(), "full", binarySubtaskLength) == 0) {
        finish(score(static_cast<int>(encodedLength)), "Correct");
    } else {
        finish(1.0, "Correct");
    }
}
