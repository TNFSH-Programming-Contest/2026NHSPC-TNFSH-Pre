#include "Cake_4.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#pragma GCC optimize("O3")

namespace {

using Matrix = std::array<std::array<long long, 2000>, 2000>;

constexpr int HEADER_BITS = 3;
constexpr int STATE_BITS = 48;
constexpr int CHUNK_BITS = 16;
constexpr std::uint64_t CHUNK_BASE = std::uint64_t{1} << CHUNK_BITS;
constexpr std::uint64_t CHUNK_MASK = CHUNK_BASE - 1;
constexpr std::uint64_t RANS_LOWER_BOUND = std::uint64_t{1} << 31;
constexpr std::uint32_t MODEL_LIMIT = 16384;
constexpr std::uint32_t RANS_SCALE_BITS = 15;
constexpr std::uint32_t RANS_TOTAL = std::uint32_t{1} << RANS_SCALE_BITS;
constexpr std::uint32_t RANS_MASK = RANS_TOTAL - 1;

long long dp[2000 * 2000];
std::uint16_t optimalCut[2000 * 2000];
std::uint16_t decodedCut[2000 * 2000];
int decodedN;

std::size_t index(int n, int l, int r) {
    return static_cast<std::size_t>(l) * n + r;
}

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

void computeCuts(int n, const Matrix& w) {
    for (int i = 0; i < n; ++i) {
        optimalCut[index(n, i, i)] = static_cast<std::uint16_t>(i);
    }
    for (int length = 2; length <= n; ++length) {
        for (int l = 0; l + length <= n; ++l) {
            const int r = l + length - 1;
            const int low = optimalCut[index(n, l, r - 1)];
            const int high =
                std::min<int>(optimalCut[index(n, l + 1, r)], r - 1);
            long long best = -1;
            int bestCut = low;
            for (int k = low; k <= high; ++k) {
                const long long candidate =
                    dp[index(n, l, k)] + dp[index(n, k + 1, r)];
                if (best < 0 || candidate < best) {
                    best = candidate;
                    bestCut = k;
                }
            }
            dp[index(n, l, r)] = best + w[l][r];
            optimalCut[index(n, l, r)] =
                static_cast<std::uint16_t>(bestCut);
        }
    }
}

struct Event {
    std::uint16_t cumulative;
    std::uint16_t frequency;
};

Event quantizedEvent(std::uint32_t cumulative,
                     std::uint32_t frequency,
                     std::uint32_t total) {
    const std::uint32_t low = static_cast<std::uint32_t>(
        static_cast<std::uint64_t>(cumulative) * RANS_TOTAL / total);
    const std::uint32_t high = static_cast<std::uint32_t>(
        static_cast<std::uint64_t>(cumulative + frequency) *
        RANS_TOTAL / total);
    return {static_cast<std::uint16_t>(low),
            static_cast<std::uint16_t>(high - low)};
}

class AdaptiveModels {
public:
    explicit AdaptiveModels(int limit)
        : limit_(limit),
          contexts_(static_cast<std::size_t>(limit + 1) * (limit + 2) / 2) {}

    bool handles(int radix) const { return radix <= limit_; }

    Event observe(int symbol, int radix, int predicted) {
        Context& context = get(radix, predicted);
        rescale(context);
        std::uint32_t cumulative = 0;
        for (int value = 0; value < symbol; ++value) {
            cumulative += context.frequency[value];
        }
        const Event result = quantizedEvent(
            cumulative, context.frequency[symbol], context.total);
        ++context.frequency[symbol];
        ++context.total;
        return result;
    }

    template <typename Decoder>
    int decode(Decoder& decoder, int radix, int predicted) {
        Context& context = get(radix, predicted);
        rescale(context);
        const std::uint32_t scaled = decoder.scaled();
        std::uint32_t cumulative = 0;
        int symbol = 0;
        Event event;
        while (true) {
            event = quantizedEvent(
                cumulative, context.frequency[symbol], context.total);
            if (scaled < event.cumulative + event.frequency) break;
            cumulative += context.frequency[symbol++];
        }
        decoder.consume(event.cumulative, event.frequency);
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
        if (context.total < MODEL_LIMIT) return;
        context.total = 0;
        for (std::uint16_t& frequency : context.frequency) {
            frequency = static_cast<std::uint16_t>((frequency + 1) / 2);
            context.total += frequency;
        }
    }

    int limit_;
    std::vector<Context> contexts_;
};

class RansEncoder {
public:
    void encode(const Event& event) {
        const std::uint64_t threshold =
            event.frequency *
            ((RANS_LOWER_BOUND >> RANS_SCALE_BITS) * CHUNK_BASE);
        while (state_ >= threshold) {
            chunks_.push_back(static_cast<std::uint16_t>(state_ & CHUNK_MASK));
            state_ >>= CHUNK_BITS;
        }
        state_ = (state_ / event.frequency) * RANS_TOTAL +
                 state_ % event.frequency + event.cumulative;
    }

    std::string finish(int mode) const {
        std::string result;
        result.reserve(HEADER_BITS + STATE_BITS + chunks_.size() * CHUNK_BITS);
        appendBits(result, static_cast<std::uint64_t>(mode), HEADER_BITS);
        appendBits(result, state_, STATE_BITS);
        for (std::uint16_t chunk : chunks_) {
            appendBits(result, chunk, CHUNK_BITS);
        }
        return result;
    }

private:
    static void appendBits(std::string& output, std::uint64_t value, int bits) {
        for (int bit = bits - 1; bit >= 0; --bit) {
            output.push_back((value >> bit) & 1 ? '1' : '0');
        }
    }

    std::uint64_t state_ = RANS_LOWER_BOUND;
    std::vector<std::uint16_t> chunks_;
};

class RansDecoder {
public:
    explicit RansDecoder(const std::string& input) {
        state_ = readBits(input, HEADER_BITS, STATE_BITS);
        for (std::size_t position = HEADER_BITS + STATE_BITS;
             position + CHUNK_BITS <= input.size();
             position += CHUNK_BITS) {
            chunks_.push_back(static_cast<std::uint16_t>(
                readBits(input, position, CHUNK_BITS)));
        }
        position_ = chunks_.size();
    }

    std::uint32_t scaled() const {
        return static_cast<std::uint32_t>(state_ & RANS_MASK);
    }

    void consume(std::uint32_t cumulative,
                 std::uint32_t frequency) {
        const std::uint32_t scaledValue =
            static_cast<std::uint32_t>(state_ & RANS_MASK);
        state_ = frequency * (state_ >> RANS_SCALE_BITS) +
                 scaledValue - cumulative;
        while (state_ < RANS_LOWER_BOUND) {
            state_ = (state_ << CHUNK_BITS) | chunks_[--position_];
        }
    }

private:
    static std::uint64_t readBits(const std::string& input,
                                  std::size_t position,
                                  int bits) {
        std::uint64_t result = 0;
        for (int bit = 0; bit < bits; ++bit) {
            result = (result << 1) | (input[position + bit] == '1');
        }
        return result;
    }

    std::uint64_t state_ = 0;
    std::vector<std::uint16_t> chunks_;
    std::size_t position_ = 0;
};

Event uniformEvent(int symbol, int radix) {
    return quantizedEvent(symbol, 1, radix);
}

std::vector<Event> collectEvents(int n, const Mode& mode) {
    std::vector<Event> events;
    events.reserve(static_cast<std::size_t>(n) * (n - 1) / 2);
    AdaptiveModels adaptive(mode.adaptiveLimit);

    auto addEvent = [&](int symbol, int radix, int predicted) {
        events.push_back(adaptive.handles(radix)
            ? adaptive.observe(symbol, radix, predicted)
            : uniformEvent(symbol, radix));
    };

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
                const int root = optimalCut[index(n, l, r)];
                const int predicted = prediction(n, l, r, low, high) - low;
                addEvent(root - low, high - low + 1, predicted);
                current[l] = root;
            }
            previous.swap(current);
        }
    } else {
        for (int l = n - 2; l >= 0; --l) {
            int previous = l;
            for (int r = l + 1; r < n; ++r) {
                const int low = previous;
                const int high = optimalCut[index(n, l + 1, r)];
                const int root = optimalCut[index(n, l, r)];
                const int predicted = prediction(n, l, r, low, high) - low;
                addEvent(root - low, high - low + 1, predicted);
                previous = root;
            }
        }
    }
    return events;
}

std::string encodeWithMode(int n, const Mode& mode, int modeIndex) {
    const std::vector<Event> events = collectEvents(n, mode);
    RansEncoder encoder;
    for (std::size_t i = events.size(); i-- > 0;) {
        encoder.encode(events[i]);
    }
    return encoder.finish(modeIndex);
}

int decodeUniform(RansDecoder& decoder, int radix) {
    const std::uint32_t scaled = decoder.scaled();
    const int symbol = static_cast<int>(
        (static_cast<std::uint64_t>(scaled + 1) * radix +
         RANS_TOTAL - 1) / RANS_TOTAL - 1);
    const Event event = uniformEvent(symbol, radix);
    decoder.consume(event.cumulative, event.frequency);
    return symbol;
}

void decodeWithMode(int n, const std::string& input, const Mode& mode) {
    RansDecoder decoder(input);
    AdaptiveModels adaptive(mode.adaptiveLimit);
    for (int i = 0; i < n; ++i) {
        decodedCut[index(n, i, i)] = static_cast<std::uint16_t>(i);
    }

    auto nextSymbol = [&](int radix, int predicted) {
        return adaptive.handles(radix)
            ? adaptive.decode(decoder, radix, predicted)
            : decodeUniform(decoder, radix);
    };

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
                const int predicted = prediction(n, l, r, low, high) - low;
                const int root = low +
                    nextSymbol(high - low + 1, predicted);
                current[l] = root;
                decodedCut[index(n, l, r)] =
                    static_cast<std::uint16_t>(root);
            }
            previous.swap(current);
        }
    } else {
        for (int l = n - 2; l >= 0; --l) {
            int previous = l;
            for (int r = l + 1; r < n; ++r) {
                const int low = previous;
                const int high = decodedCut[index(n, l + 1, r)];
                const int predicted = prediction(n, l, r, low, high) - low;
                previous = low + nextSymbol(high - low + 1, predicted);
                decodedCut[index(n, l, r)] =
                    static_cast<std::uint16_t>(previous);
            }
        }
    }
}

}  // namespace

std::string encode(int n, const Matrix& w) {
    computeCuts(n, w);
    const std::vector<Mode> allModes = modes();
    std::string best = encodeWithMode(n, allModes[0], 0);
    for (int mode = 1; mode < static_cast<int>(allModes.size()); ++mode) {
        std::string candidate = encodeWithMode(n, allModes[mode], mode);
        if (candidate.size() < best.size()) best.swap(candidate);
    }
    return best;
}

void decode(int n, const std::string& input) {
    decodedN = n;
    int mode = 0;
    for (int bit = 0; bit < HEADER_BITS; ++bit) {
        mode = (mode << 1) | (input[bit] == '1');
    }
    const std::vector<Mode> allModes = modes();
    decodeWithMode(n, input, allModes[mode]);
}

int query(int l, int r) {
    return decodedCut[index(decodedN, l, r)];
}
