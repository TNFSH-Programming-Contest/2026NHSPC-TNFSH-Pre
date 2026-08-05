#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr std::size_t BIT_LIMIT = 4000000;
std::ofstream managerLog;

[[noreturn]] void finish(double score, const std::string& message) {
    std::cout << score << '\n';
    std::cerr << message << '\n';
    if (managerLog) managerLog << message << '\n';
    std::exit(0);
}

[[noreturn]] void wrong(const std::string& message) {
    finish(0.0, message);
}

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

struct OptimalCuts {
    int n;
    std::vector<long long> dp;
    std::vector<int> opt;

    explicit OptimalCuts(const std::vector<std::vector<long long>>& w)
        : n(static_cast<int>(w.size())),
          dp(static_cast<std::size_t>(n) * n),
          opt(static_cast<std::size_t>(n) * n) {
        for (int i = 0; i < n; ++i) {
            opt[index(i, i)] = i;
        }
        for (int length = 2; length <= n; ++length) {
            for (int l = 0; l + length <= n; ++l) {
                const int r = l + length - 1;
                int low = std::max(opt[index(l, r - 1)], l);
                int high = std::min(opt[index(l + 1, r)], r - 1);
                long long best = std::numeric_limits<long long>::max();
                int bestCut = low;
                for (int k = low; k <= high; ++k) {
                    const long long candidate =
                        dp[index(l, k)] + dp[index(k + 1, r)];
                    if (candidate < best) {
                        best = candidate;
                        bestCut = k;
                    }
                }
                dp[index(l, r)] = best + w[l][r];
                opt[index(l, r)] = bestCut;
            }
        }
    }

    std::size_t index(int l, int r) const {
        return static_cast<std::size_t>(l) * n + r;
    }

    bool isOptimal(int l, int r, int k, long long intervalCost) const {
        const long long candidate =
            dp[index(l, k)] + dp[index(k + 1, r)];
        return candidate == dp[index(l, r)] - intervalCost;
    }
};

}  // namespace

int main(int argc, char* argv[]) {
    std::signal(SIGPIPE, SIG_IGN);
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    if (argc < 5) wrong("Judge Error: manager received too few FIFO arguments");
    if (argc >= 6) managerLog.open(argv[5]);

    std::ifstream fromEncoder(argv[1]);
    std::ofstream toEncoder(argv[2]);
    std::ifstream fromDecoder(argv[3]);
    std::ofstream toDecoder(argv[4]);
    if (!fromEncoder || !toEncoder || !fromDecoder || !toDecoder) {
        wrong("Judge Error: cannot open communication FIFO");
    }

    int n, q;
    long long scale;
    if (!(std::cin >> n >> q >> scale)) {
        wrong("Judge Error: invalid testcase header");
    }
    std::vector<std::vector<long long>> w(
        n, std::vector<long long>(n));
    std::vector<long long> delta(n);
    for (int r = 1; r < n; ++r) {
        for (int l = 0; l < r; ++l) {
            if (!(std::cin >> delta[l])) {
                wrong("Judge Error: invalid interval-cost differences");
            }
        }
        long long extensionCost = 1;
        for (int l = r - 1; l >= 0; --l) {
            extensionCost += delta[l];
            w[l][r] =
                w[l][r - 1] + extensionCost * scale;
        }
    }
    std::vector<std::pair<int, int>> queries(q);
    for (auto& item : queries) {
        if (!(std::cin >> item.first >> item.second)) {
            wrong("Judge Error: invalid query list");
        }
    }

    const OptimalCuts optimal(w);

    toEncoder << n << '\n';
    for (int l = 0; l < n; ++l) {
        for (int r = l; r < n; ++r) {
            if (r != l) toEncoder << ' ';
            toEncoder << w[l][r];
        }
        toEncoder << '\n';
    }
    toEncoder.flush();
    if (!toEncoder) wrong("Wrong Answer: encoder stopped before reading input");
    toEncoder.close();

    unsigned long long encodedLength;
    if (!(fromEncoder >> encodedLength)) {
        wrong("Wrong Answer (1): encoder did not return a string");
    }
    if (encodedLength > BIT_LIMIT) {
        wrong("Wrong Answer (1): encoded string is longer than B");
    }

    std::string encoded;
    if (!readRawString(fromEncoder, static_cast<std::size_t>(encodedLength),
                       encoded)) {
        wrong("Wrong Answer (1): malformed encoded string");
    }
    if (!std::all_of(encoded.begin(), encoded.end(),
                     [](char c) { return c == '0' || c == '1'; })) {
        wrong("Wrong Answer (1): encoded string is not binary");
    }
    fromEncoder.close();

    toDecoder << n << ' ' << encoded.size() << '\n';
    toDecoder.write(encoded.data(),
                    static_cast<std::streamsize>(encoded.size()));
    toDecoder << '\n' << q << '\n';
    for (const auto& item : queries) {
        toDecoder << item.first << ' ' << item.second << '\n';
    }
    toDecoder.flush();
    if (!toDecoder) wrong("Wrong Answer: decoder stopped before reading input");
    toDecoder.close();

    int firstBadType = 0;
    int firstBadQuery = -1;
    int firstBadAnswer = 0;
    for (int i = 0; i < q; ++i) {
        int answer;
        if (!(fromDecoder >> answer)) {
            wrong("Wrong Answer: decoder did not answer every query");
        }
        const int l = queries[i].first;
        const int r = queries[i].second;
        if (firstBadType == 0 && !(l <= answer && answer < r)) {
            firstBadType = 2;
            firstBadQuery = i;
            firstBadAnswer = answer;
        } else if (firstBadType == 0 &&
                   !optimal.isOptimal(l, r, answer, w[l][r])) {
            firstBadType = 3;
            firstBadQuery = i;
            firstBadAnswer = answer;
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
    finish(1.0, "Correct");
}
