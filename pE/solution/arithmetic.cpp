#include "Cake_4.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#pragma GCC optimize("O3")

namespace {

using Matrix = std::array<std::array<long long, 2000>, 2000>;
long long dp[2000 * 2000];
uint16_t opt[2000 * 2000];

constexpr std::uint64_t HALF = std::uint64_t{1} << 31;
constexpr std::uint64_t FIRST_QUARTER = std::uint64_t{1} << 30;
constexpr std::uint64_t THIRD_QUARTER = FIRST_QUARTER * 3;
constexpr int HEADER_BITS = 3;

int decodedN;
uint16_t decodedCuts[2000 * 2000];

std::size_t index(int n, int l, int r) {
    return static_cast<std::size_t>(l) * n + r;
}

class ArithmeticEncoder {
public:
    void encode(std::uint32_t cumulativeLow,
                std::uint32_t cumulativeHigh,
                std::uint32_t total) {
        const std::uint64_t width = high_ - low_ + 1;
        high_ = low_ + width * cumulativeHigh / total - 1;
        low_ += width * cumulativeLow / total;

        while (true) {
            if (high_ < HALF) {
                outputWithPending(false);
            } else if (low_ >= HALF) {
                outputWithPending(true);
                low_ -= HALF;
                high_ -= HALF;
            } else if (low_ >= FIRST_QUARTER && high_ < THIRD_QUARTER) {
                ++pending_;
                low_ -= FIRST_QUARTER;
                high_ -= FIRST_QUARTER;
            } else {
                break;
            }
            low_ <<= 1;
            high_ = (high_ << 1) | 1;
        }
    }

    std::string finish() {
        ++pending_;
        outputWithPending(low_ >= FIRST_QUARTER);
        return std::move(output_);
    }

private:
    void outputWithPending(bool bit) {
        output_.push_back(bit ? '1' : '0');
        output_.append(pending_, bit ? '0' : '1');
        pending_ = 0;
    }

    std::uint64_t low_ = 0;
    std::uint64_t high_ = (std::uint64_t{1} << 32) - 1;
    std::size_t pending_ = 0;
    std::string output_;
};

class ArithmeticDecoder {
public:
    ArithmeticDecoder(const std::string& input, std::size_t position)
        : input_(input), position_(position) {
        for (int i = 0; i < 32; ++i) code_ = (code_ << 1) | readBit();
    }

    std::uint32_t scaled(std::uint32_t total) const {
        const std::uint64_t width = high_ - low_ + 1;
        return static_cast<std::uint32_t>(
            ((code_ - low_ + 1) * total - 1) / width);
    }

    void consume(std::uint32_t cumulativeLow,
                 std::uint32_t cumulativeHigh,
                 std::uint32_t total) {
        const std::uint64_t width = high_ - low_ + 1;
        high_ = low_ + width * cumulativeHigh / total - 1;
        low_ += width * cumulativeLow / total;

        while (true) {
            if (high_ < HALF) {
                // Nothing to subtract.
            } else if (low_ >= HALF) {
                code_ -= HALF;
                low_ -= HALF;
                high_ -= HALF;
            } else if (low_ >= FIRST_QUARTER && high_ < THIRD_QUARTER) {
                code_ -= FIRST_QUARTER;
                low_ -= FIRST_QUARTER;
                high_ -= FIRST_QUARTER;
            } else {
                break;
            }
            low_ <<= 1;
            high_ = (high_ << 1) | 1;
            code_ = (code_ << 1) | readBit();
        }
    }

private:
    std::uint32_t readBit() {
        if (position_ >= input_.size()) return 0;
        return input_[position_++] == '1';
    }

    const std::string& input_;
    std::size_t position_;
    std::uint64_t low_ = 0;
    std::uint64_t high_ = (std::uint64_t{1} << 32) - 1;
    std::uint64_t code_ = 0;
};

struct Mode {
    bool rowOrder;
    int adaptiveLimit;
};

std::vector<Mode> modes() {
    std::vector<Mode> result;
    for (int rowOrder = 0; rowOrder <= 1; ++rowOrder) {
        for (int adaptiveLimit : {/*0, 8,*/ 16, 32}) {
            result.push_back({rowOrder != 0, adaptiveLimit});
        }
    }
    return result;
}

int prediction(int n, int l, int r, int low, int high) {
    const int remaining = n - (r - l) - 1;
    int result;
    if (remaining > 0) {
        result = static_cast<int>(
            static_cast<long long>(l) * (n - 2) / remaining);
    } else {
        result = (low + high) / 2;
    }
    return std::max(low, std::min(high, result));
}

void encodeUniform(ArithmeticEncoder& coder, int symbol, int radix) {
    coder.encode(symbol, symbol + 1, radix);
}

int decodeUniform(ArithmeticDecoder& coder, int radix) {
    const int symbol = static_cast<int>(coder.scaled(radix));
    coder.consume(symbol, symbol + 1, radix);
    return symbol;
}

class AdaptiveModels {
public:
    explicit AdaptiveModels(int limit)
        : limit_(limit), contexts_((limit + 1) * (limit + 2) / 2) {}

    bool handles(int radix) const { return radix <= limit_; }

    void encode(ArithmeticEncoder& coder, int symbol, int radix,
                int predicted) {
        Context& context = get(radix, predicted);
        rescale(context);
        std::uint32_t cumulative = 0;
        for (int value = 0; value < symbol; ++value) {
            cumulative += context.frequency[value];
        }
        coder.encode(cumulative, cumulative + context.frequency[symbol],
                     context.total);
        ++context.frequency[symbol];
        ++context.total;
    }

    int decode(ArithmeticDecoder& coder, int radix, int predicted) {
        Context& context = get(radix, predicted);
        rescale(context);
        const std::uint32_t scaled = coder.scaled(context.total);
        std::uint32_t cumulative = 0;
        int symbol = 0;
        while (cumulative + context.frequency[symbol] <= scaled) {
            cumulative += context.frequency[symbol++];
        }
        coder.consume(cumulative, cumulative + context.frequency[symbol],
                      context.total);
        ++context.frequency[symbol];
        ++context.total;
        return symbol;
    }

private:
    struct Context {
        std::vector<std::uint16_t> frequency;
        std::uint32_t total = 0;
    };

    Context& get(int radix, int predicted) {
        Context& context =
            contexts_[static_cast<std::size_t>(radix) * (radix - 1) / 2 +
                      predicted];
        if (context.total == 0) {
            context.frequency.assign(radix, 1);
            context.total = radix;
        }
        return context;
    }

    void rescale(Context& context) {
        if (context.total < 16384) return;
        context.total = 0;
        for (std::uint16_t& value : context.frequency) {
            value = static_cast<std::uint16_t>((value + 1) / 2);
            context.total += value;
        }
    }

    int limit_;
    std::vector<Context> contexts_;
};


void computeCuts(int n, const Matrix& w) {
    for (int i = 0; i < n; ++i) opt[index(n, i, i)] = i;

    for (int length = 2; length <= n; ++length) {
        for (int l = 0; l + length <= n; ++l) {
            const int r = l + length - 1;
            const int low = opt[index(n, l, r - 1)];
            const int high = std::min<int>(opt[index(n, l + 1, r)], r - 1);
            long long best = -1;
            for (int k = low; k <= high; ++k) {
                const long long candidate =
                    dp[index(n, l, k)] + dp[index(n, k + 1, r)];
                if (best < 0 || candidate < best) {
                    best = candidate;
                    opt[index(n, l, r)] = static_cast<std::uint16_t>(k);
                }
            }
            dp[index(n, l, r)] = best + w[l][r];
        }
    }
}

std::string encodeWithMode(int n, const Mode& mode) {
    ArithmeticEncoder coder;
    AdaptiveModels adaptive(mode.adaptiveLimit);
    if (!mode.rowOrder) {
        std::vector<int> previous(n);
        for (int l = 0; l < n; ++l) previous[l] = l;
        for (int d = 1; d < n; ++d) {
            const int length = n - d;
            std::vector<int> current(length);
            for (int l = 0; l < length; ++l) {
                const int r = l + d;
                const int low = previous[l];
                const int high = previous[l + 1];
                const int predictedRoot =
                    prediction(n, l, r, low, high);
                const int symbol = opt[index(n, l, r)] - low;
                const int radix = high - low + 1;
                if (adaptive.handles(radix)) {
                    adaptive.encode(coder, symbol, radix, predictedRoot - low);
                } else {
                    encodeUniform(coder, symbol, radix);
                }
                current[l] = opt[index(n, l, r)];
            }
            previous.swap(current);
        }
    } else {
        for (int l = n - 2; l >= 0; --l) {
            int previous = l;
            for (int r = l + 1; r < n; ++r) {
                const int low = previous;
                const int high = opt[index(n, l + 1, r)];
                const int predictedRoot =
                    prediction(n, l, r, low, high);
                const int symbol = opt[index(n, l, r)] - low;
                const int radix = high - low + 1;
                if (adaptive.handles(radix)) {
                    adaptive.encode(coder, symbol, radix, predictedRoot - low);
                } else {
                    encodeUniform(coder, symbol, radix);
                }
                previous = opt[index(n, l, r)];
            }
        }
    }
    return coder.finish();
}

void decodeWithMode(int n, const std::string& input, const Mode& mode) {
    ArithmeticDecoder coder(input, HEADER_BITS);
    AdaptiveModels adaptive(mode.adaptiveLimit);
    for (int i = 0; i < n; ++i) decodedCuts[index(n, i, i)] = i;

    if (!mode.rowOrder) {
        std::vector<int> previous(n);
        for (int l = 0; l < n; ++l) previous[l] = l;
        for (int d = 1; d < n; ++d) {
            const int length = n - d;
            std::vector<int> current(length);
            for (int l = 0; l < length; ++l) {
                const int r = l + d;
                const int low = previous[l];
                const int high = previous[l + 1];
                const int predictedRoot =
                    prediction(n, l, r, low, high);
                const int radix = high - low + 1;
                const int symbol = adaptive.handles(radix)
                    ? adaptive.decode(coder, radix, predictedRoot - low)
                    : decodeUniform(coder, radix);
                current[l] = low + symbol;
                decodedCuts[index(n, l, r)] = current[l];
            }
            previous.swap(current);
        }
    } else {
        for (int l = n - 2; l >= 0; --l) {
            int previous = l;
            for (int r = l + 1; r < n; ++r) {
                const int low = previous;
                const int high = decodedCuts[index(n, l + 1, r)];
                const int predictedRoot =
                    prediction(n, l, r, low, high);
                const int radix = high - low + 1;
                const int symbol = adaptive.handles(radix)
                    ? adaptive.decode(coder, radix, predictedRoot - low)
                    : decodeUniform(coder, radix);
                previous = low + symbol;
                decodedCuts[index(n, l, r)] = previous;
            }
        }
    }
}

}  // namespace

std::string encode(int n, const Matrix& w) {
    computeCuts(n, w);
    const std::vector<Mode> allModes = modes();

    int bestMode = 0;
    std::string best = encodeWithMode(n, allModes[0]);
    for (int mode = 1; mode < static_cast<int>(allModes.size()); ++mode) {
        std::string candidate = encodeWithMode(n, allModes[mode]);
        if (candidate.size() < best.size()) {
            bestMode = mode;
            best.swap(candidate);
        }
    }

    std::string result;
    result.reserve(HEADER_BITS + best.size());
    for (int bit = HEADER_BITS - 1; bit >= 0; --bit) {
        result.push_back((bestMode >> bit) & 1 ? '1' : '0');
    }
    result += best;
    return result;
}

void decode(int n, const std::string& s) {
    decodedN = n;
    int mode = 0;
    for (int bit = 0; bit < HEADER_BITS; ++bit) {
        mode = (mode << 1) | (s[bit] == '1');
    }
    const std::vector<Mode> allModes = modes();
    decodeWithMode(n, s, allModes[mode]);
}

int query(int l, int r) {
    return decodedCuts[index(decodedN, l, r)];
}
