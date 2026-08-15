#include "Cake_4.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
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

constexpr std::size_t MAX_N = 2000;
constexpr std::size_t MAX_INTERVALS = MAX_N * (MAX_N + 1) / 2;

long long dp[MAX_INTERVALS];
std::uint16_t optimalCut[MAX_INTERVALS];
std::uint16_t decodedCut[MAX_INTERVALS];
int decodedN;

std::size_t index(int n, int l, int r) {
    // Row-major storage of only (l, r) with l <= r.
    return static_cast<std::size_t>(l) * n -
           static_cast<std::size_t>(l) * (l + 1) / 2 + r;
}

struct Mode {
    bool rowOrder;
    int adaptiveLimit;
};

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

    void update(int symbol, int radix, int predicted) {
        Context& context = get(radix, predicted);
        rescale(context);
        ++context.frequency[symbol];
        ++context.total;
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

#include <limits>

namespace {

constexpr int MIN_MATCH = 16;
constexpr int MAX_MATCH = 1024;
constexpr int WINDOW_SIZE = 65535;
constexpr int DISTANCE_BASE = 256;
constexpr int HASH_PREFIX = 4;
constexpr int HASH_BITS = 20;
constexpr int HASH_SIZE = 1 << HASH_BITS;

struct SymbolInfo {
    std::uint16_t digit;
    std::uint16_t radix;
    std::uint16_t predicted;
};

struct Token {
    std::uint32_t position;
    std::uint16_t length;
    std::uint16_t distance;

    bool isMatch() const { return distance != 0; }
};

struct DictionaryMode {
    bool rowOrder;
    int adaptiveLimit;
    bool dictionary;
};

std::vector<DictionaryMode> dictionaryModes() {
    return {
        {false, 0, false},
        {false, 32, false},
        {false, 8, true},
        {false, 32, true},
        {true, 0, false},
        {true, 32, false},
        {true, 8, true},
        {true, 32, true}
    };
}

std::vector<SymbolInfo> collectSymbols(int n, bool rowOrder) {
    std::vector<SymbolInfo> symbols;
    symbols.reserve(static_cast<std::size_t>(n) * (n - 1) / 2);
    if (!rowOrder) {
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
                symbols.push_back({
                    static_cast<std::uint16_t>(root - low),
                    static_cast<std::uint16_t>(high - low + 1),
                    static_cast<std::uint16_t>(
                        prediction(n, l, r, low, high) - low)
                });
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
                symbols.push_back({
                    static_cast<std::uint16_t>(root - low),
                    static_cast<std::uint16_t>(high - low + 1),
                    static_cast<std::uint16_t>(
                        prediction(n, l, r, low, high) - low)
                });
                previous = root;
            }
        }
    }
    return symbols;
}

std::uint32_t hashDigits(const std::vector<SymbolInfo>& symbols,
                         std::size_t position) {
    std::uint64_t value = 0x9e3779b97f4a7c15ULL;
    for (int i = 0; i < HASH_PREFIX; ++i) {
        value ^= symbols[position + i].digit + 0x9e3779b9U +
                 (value << 6) + (value >> 2);
    }
    value ^= value >> 30;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27;
    return static_cast<std::uint32_t>(value) & (HASH_SIZE - 1);
}

std::vector<Token> makeTokens(const std::vector<SymbolInfo>& symbols) {
    const std::size_t size = symbols.size();
    std::vector<int> head(HASH_SIZE, -1);
    std::vector<Token> tokens;
    // Only matches are stored. Literals are implicit gaps between them.
    tokens.reserve(size / (MIN_MATCH * 4));

    const auto insert = [&](std::size_t position) {
        if (position + MIN_MATCH > size) return;
        const std::uint32_t hash = hashDigits(symbols, position);
        head[hash] = static_cast<int>(position);
    };

    std::size_t position = 0;
    while (position < size) {
        int bestLength = 0;
        int bestDistance = 0;
        if (position + MIN_MATCH <= size) {
            const std::uint32_t hash = hashDigits(symbols, position);
            const int candidate = head[hash];
            if (candidate >= 0) {
                const int distance =
                    static_cast<int>(position) - candidate;
                if (distance <= WINDOW_SIZE) {
                    const int maximum = static_cast<int>(
                        std::min<std::size_t>(MAX_MATCH, size - position));
                    int length = 0;
                    while (length < maximum &&
                           symbols[candidate + length].digit ==
                               symbols[position + length].digit) {
                        ++length;
                    }
                    if (length >= MIN_MATCH) {
                        bestLength = length;
                        bestDistance = distance;
                    }
                }
            }
        }

        if (bestLength >= MIN_MATCH) {
            tokens.push_back({static_cast<std::uint32_t>(position),
                              static_cast<std::uint16_t>(bestLength),
                              static_cast<std::uint16_t>(bestDistance)});
            for (int i = 0; i < bestLength; ++i) insert(position + i);
            position += bestLength;
        } else {
            insert(position);
            ++position;
        }
    }
    return tokens;
}

int flagContext(const SymbolInfo& symbol) {
    return symbol.radix <= 2 ? 0 : 1;
}

Event observeDigit(AdaptiveModels& models, const SymbolInfo& symbol) {
    return models.handles(symbol.radix)
        ? models.observe(symbol.digit, symbol.radix, symbol.predicted)
        : uniformEvent(symbol.digit, symbol.radix);
}

std::string encodeDictionary(const std::vector<SymbolInfo>& symbols,
                             const std::vector<Token>& tokens,
                             int adaptiveLimit,
                             int modeIndex) {
    AdaptiveModels digitModels(adaptiveLimit);
    AdaptiveModels flagModels(2);
    std::vector<Event> events;
    events.reserve(symbols.size());

    std::size_t tokenIndex = 0;
    std::size_t position = 0;
    while (position < symbols.size()) {
        const bool isMatch = tokenIndex < tokens.size() &&
            tokens[tokenIndex].position == position;
        const std::size_t remaining = symbols.size() - position;
        const bool canMatch = position > 0 && remaining >= MIN_MATCH;
        if (canMatch) {
            events.push_back(flagModels.observe(
                isMatch, 2, flagContext(symbols[position])));
        }
        if (!isMatch) {
            events.push_back(observeDigit(digitModels, symbols[position]));
            ++position;
            continue;
        }

        const Token& token = tokens[tokenIndex++];
        const int maximumLength =
            static_cast<int>(std::min<std::size_t>(MAX_MATCH, remaining));
        events.push_back(uniformEvent(
            token.length - MIN_MATCH, maximumLength - MIN_MATCH + 1));
        const int maximumDistance =
            std::min<int>(static_cast<int>(position), WINDOW_SIZE);
        const int distanceValue = token.distance - 1;
        const int distanceBucket = distanceValue / DISTANCE_BASE;
        const int bucketCount =
            (maximumDistance + DISTANCE_BASE - 1) / DISTANCE_BASE;
        events.push_back(uniformEvent(distanceBucket, bucketCount));
        const int offsetRadix = std::min(
            DISTANCE_BASE,
            maximumDistance - distanceBucket * DISTANCE_BASE);
        events.push_back(uniformEvent(
            distanceValue % DISTANCE_BASE, offsetRadix));
        for (int i = 0; i < token.length; ++i) {
            if (digitModels.handles(symbols[position + i].radix)) {
                digitModels.update(
                    symbols[position + i].digit,
                    symbols[position + i].radix,
                    symbols[position + i].predicted);
            }
        }
        position += token.length;
    }

    RansEncoder encoder;
    for (std::size_t i = events.size(); i-- > 0;) {
        encoder.encode(events[i]);
    }
    return encoder.finish(modeIndex);
}

class DictionaryDigitSource {
public:
    DictionaryDigitSource(const std::string& input,
                          int adaptiveLimit,
                          std::size_t totalSymbols)
        : decoder_(input),
          digitModels_(adaptiveLimit),
          flagModels_(2),
          totalSymbols_(totalSymbols) {
        history_.reserve(totalSymbols);
    }

    int next(int radix, int predicted) {
        const SymbolInfo symbol = {
            0, static_cast<std::uint16_t>(radix),
            static_cast<std::uint16_t>(predicted)
        };
        if (matchRemaining_ == 0) {
            const std::size_t remaining = totalSymbols_ - history_.size();
            const bool canMatch = !history_.empty() && remaining >= MIN_MATCH;
            const int isMatch = canMatch
                ? flagModels_.decode(
                    decoder_, 2, flagContext(symbol))
                : 0;
            if (!isMatch) {
                const int digit = digitModels_.handles(radix)
                    ? digitModels_.decode(decoder_, radix, predicted)
                    : decodeUniform(decoder_, radix);
                history_.push_back(static_cast<std::uint16_t>(digit));
                return digit;
            }

            const int maximumLength = static_cast<int>(
                std::min<std::size_t>(MAX_MATCH, remaining));
            matchRemaining_ = MIN_MATCH + decodeUniform(
                decoder_, maximumLength - MIN_MATCH + 1);
            const int maximumDistance = std::min<int>(
                static_cast<int>(history_.size()), WINDOW_SIZE);
            const int bucketCount =
                (maximumDistance + DISTANCE_BASE - 1) / DISTANCE_BASE;
            const int distanceBucket = decodeUniform(decoder_, bucketCount);
            const int offsetRadix = std::min(
                DISTANCE_BASE,
                maximumDistance - distanceBucket * DISTANCE_BASE);
            matchDistance_ = 1 + distanceBucket * DISTANCE_BASE +
                decodeUniform(decoder_, offsetRadix);
        }

        const int digit = history_[history_.size() - matchDistance_];
        history_.push_back(static_cast<std::uint16_t>(digit));
        --matchRemaining_;
        if (digitModels_.handles(radix)) {
            digitModels_.update(digit, radix, predicted);
        }
        return digit;
    }

private:
    RansDecoder decoder_;
    AdaptiveModels digitModels_;
    AdaptiveModels flagModels_;
    std::size_t totalSymbols_;
    std::vector<std::uint16_t> history_;
    int matchRemaining_ = 0;
    int matchDistance_ = 0;
};

void decodeDictionary(int n,
                      const std::string& input,
                      const DictionaryMode& mode) {
    const std::size_t totalSymbols =
        static_cast<std::size_t>(n) * (n - 1) / 2;
    DictionaryDigitSource source(input, mode.adaptiveLimit, totalSymbols);
    for (int i = 0; i < n; ++i) {
        decodedCut[index(n, i, i)] = static_cast<std::uint16_t>(i);
    }

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
                    source.next(high - low + 1, predicted);
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
                previous = low + source.next(high - low + 1, predicted);
                decodedCut[index(n, l, r)] =
                    static_cast<std::uint16_t>(previous);
            }
        }
    }
}

}  // namespace

std::string encode(int n, const Matrix& w) {
    computeCuts(n, w);
    const std::vector<DictionaryMode> allModes = dictionaryModes();
    // A single strong raw model is the fallback.  Trying every combination
    // costs more time than the small gain is worth on two million symbols.
    std::string best = encodeWithMode(n, {false, 32}, 1);
    if (n < 1500) {
        return best;
    }

    // The diagonal order exposes repeated local patterns while preserving the
    // Knuth interval structure.  LZ matching uses a four-symbol hash and then
    // verifies at least MIN_MATCH symbols, just like a small zstd-style parser.
    const std::vector<SymbolInfo> symbols = collectSymbols(n, false);
    const std::vector<Token> tokens = makeTokens(symbols);
    std::size_t matchedSymbols = 0;
    for (const Token& token : tokens) matchedSymbols += token.length;
    if (matchedSymbols * 5 < symbols.size()) return best;

    const int modeIndex = 3;
    std::string candidate = encodeDictionary(
        symbols, tokens, allModes[modeIndex].adaptiveLimit, modeIndex);
    if (candidate.size() < best.size()) {
        best.swap(candidate);
    }
    return best;
}

void decode(int n, const std::string& input) {
    decodedN = n;
    int modeIndex = 0;
    for (int bit = 0; bit < HEADER_BITS; ++bit) {
        modeIndex = (modeIndex << 1) | (input[bit] == '1');
    }
    const DictionaryMode mode = dictionaryModes()[modeIndex];
    if (mode.dictionary) {
        decodeDictionary(n, input, mode);
    } else {
        decodeWithMode(n, input, {mode.rowOrder, mode.adaptiveLimit});
    }
}

int query(int l, int r) {
    return decodedCut[index(decodedN, l, r)];
}
