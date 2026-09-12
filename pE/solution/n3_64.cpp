#include "Cake_4.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#ifndef FAST_CODEC_CORE_H
#define FAST_CODEC_CORE_H

#if defined(__GNUC__)
#pragma GCC optimize("O3,unroll-loops")
#pragma GCC target("avx2,bmi2,popcnt")
#endif

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <immintrin.h>
#include <memory>
#include <string>
#include <vector>

namespace fastcake {

constexpr int kBits = 2048;
constexpr int kWords = kBits / 64;
using Row = std::array<std::uint64_t, kWords>;
using Matrix = std::array<Row, kBits>;

inline bool bit(const Row& row, int position) {
    return (row[position >> 6] >> (position & 63)) & 1;
}

inline void setBit(Row& row, int position) {
    row[position >> 6] |= std::uint64_t{1} << (position & 63);
}

inline void clearOutside(Row& row, int bitCount) {
    const int words = (bitCount + 63) / 64;
    if (words && (bitCount & 63)) {
        row[words - 1] &=
            (std::uint64_t{1} << (bitCount & 63)) - 1;
    }
    for (int word = words; word < kWords; ++word) {
        row[word] = 0;
    }
}

struct ByteSummary {
    std::uint8_t farClosed;
    std::uint8_t farOpen;
    std::uint8_t closedCount;
};

struct MidEntry {
    std::uint8_t x;
    std::uint8_t covered;
};

struct ParenSummary {
    std::uint16_t farClosed;
    std::uint8_t farOpen;
    std::uint8_t closedCount;
};

class Tables {
public:
    std::array<ByteSummary, 1 << 16> summary{};
    std::array<std::array<std::uint8_t, 9>, 256> dropLowest{};
    std::array<std::array<std::uint8_t, 9>, 256> takeLowest{};
    std::array<std::uint8_t, 256> reverseByte{};
    std::array<std::array<std::uint16_t, 256>, 256> ternary{};
    std::vector<MidEntry> middle;

    Tables() : middle(6561 * 256) {
        for (int x = 0; x < 256; ++x) {
            int reversed = 0;
            for (int b = 0; b < 8; ++b) {
                reversed |= ((x >> b) & 1) << (7 - b);
            }
            reverseByte[x] = static_cast<std::uint8_t>(reversed);

            for (int count = 0; count <= 8; ++count) {
                int rest = x;
                int chosen = 0;
                for (int step = 0; step < count && rest; ++step) {
                    const int one = rest & -rest;
                    chosen |= one;
                    rest ^= one;
                }
                takeLowest[x][count] =
                    static_cast<std::uint8_t>(chosen);
                dropLowest[x][count] =
                    static_cast<std::uint8_t>(rest);
            }
        }

        for (int open = 0; open < 256; ++open) {
            for (int close = 0; close < 256; ++close) {
                int balance = 0;
                int farClosed = 0;
                for (int b = 0; b < 8; ++b) {
                    if ((open >> b) & 1) {
                        ++balance;
                    }
                    if ((close >> b) & 1) {
                        if (balance) {
                            --balance;
                        } else {
                            farClosed |= 1 << b;
                        }
                    }
                }
                summary[open | (close << 8)] = {
                    static_cast<std::uint8_t>(farClosed),
                    static_cast<std::uint8_t>(balance),
                    static_cast<std::uint8_t>(
                        __builtin_popcount(farClosed))};
            }
        }

        for (int freeBits = 0; freeBits < 256; ++freeBits) {
            for (int y = 0; y < 256; ++y) {
                int code = 0;
                int power = 1;
                for (int b = 0; b < 8; ++b) {
                    int digit = 0;
                    if ((freeBits >> b) & 1) {
                        digit = ((y >> b) & 1) ? 2 : 1;
                    }
                    code += digit * power;
                    power *= 3;
                }
                ternary[freeBits][y] =
                    static_cast<std::uint16_t>(code);
            }
        }

        for (int freeBits = 0; freeBits < 256; ++freeBits) {
            for (int y = 0; y < 256; ++y) {
                if (y & ~freeBits) {
                    continue;
                }

                const int code = ternary[freeBits][y];

                for (int boundaries = 0;
                     boundaries < 256;
                     ++boundaries) {
                    int xout = 0;
                    int covered = 0;
                    int starts = boundaries;

                    while (starts) {
                        const int begin = __builtin_ctz(starts);
                        starts &= starts - 1;
                        if (!starts) {
                            break;
                        }

                        const int end = __builtin_ctz(starts);
                        const int mask =
                            ((1 << end) - 1) &
                            ~((1 << begin) - 1);
                        const int positions = freeBits & mask;
                        const int countY =
                            __builtin_popcount(y & mask);

                        xout |= takeLowest[positions][countY];
                        covered |= positions;
                    }

                    middle[code * 256 + boundaries] = {
                        static_cast<std::uint8_t>(xout),
                        static_cast<std::uint8_t>(covered)};
                }
            }
        }
    }
};

inline Tables& tables() {
    static Tables instance;
    return instance;
}

class ParenTables {
public:
    std::array<ParenSummary, 1 << 16> full{};
    std::array<ParenSummary, 1 << 16> prefix{};

    ParenTables() {
        auto make = [](unsigned closeBits, int length) {
            int balance = 0;
            unsigned farClosed = 0;

            for (int bit = 0; bit < length; ++bit) {
                if ((closeBits >> bit) & 1) {
                    if (balance) {
                        --balance;
                    } else {
                        farClosed |= 1U << bit;
                    }
                } else {
                    ++balance;
                }
            }

            return ParenSummary{
                static_cast<std::uint16_t>(farClosed),
                static_cast<std::uint8_t>(balance),
                static_cast<std::uint8_t>(
                    __builtin_popcount(farClosed))};
        };

        for (unsigned bits = 0; bits < (1U << 16); ++bits) {
            full[bits] = make(bits, 16);
        }

        prefix[1] = make(0, 0);

        for (int length = 1; length < 16; ++length) {
            for (unsigned bits = 0;
                 bits < (1U << length);
                 ++bits) {
                prefix[(1U << length) | bits] =
                    make(bits, length);
            }
        }
    }
};

inline ParenTables& parenTables() {
    static ParenTables instance;
    return instance;
}

inline std::uint64_t reverseBits(std::uint64_t value) {
    value =
        ((value >> 1) & 0x5555555555555555ULL) |
        ((value & 0x5555555555555555ULL) << 1);
    value =
        ((value >> 2) & 0x3333333333333333ULL) |
        ((value & 0x3333333333333333ULL) << 2);
    value =
        ((value >> 4) & 0x0F0F0F0F0F0F0F0FULL) |
        ((value & 0x0F0F0F0F0F0F0F0FULL) << 4);
    return __builtin_bswap64(value);
}

inline std::uint64_t matchPackedWord(
    std::uint64_t openInput,
    std::uint64_t closeInput,
    int& balance) {
    const std::uint64_t equality = openInput & closeInput;
    const std::uint64_t open = openInput & ~closeInput;
    const std::uint64_t close = closeInput & ~openInput;
    const std::uint64_t symbols = open | close;

    const int length = __builtin_popcountll(symbols);
    const std::uint64_t packedClose =
        _pext_u64(close, symbols);

    std::uint64_t unmatchedPacked = 0;
    ParenTables& table = parenTables();

    int offset = 0;

    while (offset + 16 <= length) {
        const unsigned bits =
            static_cast<unsigned>(packedClose >> offset) &
            0xFFFFU;
        const ParenSummary summary = table.full[bits];

        const int consumed =
            std::min(balance,
                     static_cast<int>(summary.closedCount));

        const unsigned source =
            consumed == 16
                ? 0xFFFFU
                : ((1U << consumed) - 1);

        const unsigned used =
            _pdep_u32(source, summary.farClosed);

        unmatchedPacked |=
            static_cast<std::uint64_t>(
                summary.farClosed ^ used)
            << offset;

        balance =
            summary.farOpen +
            std::max(0, balance - summary.closedCount);

        offset += 16;
    }

    if (offset < length) {
        const int remaining = length - offset;
        const unsigned mask = (1U << remaining) - 1;
        const unsigned bits =
            static_cast<unsigned>(packedClose >> offset) &
            mask;

        const ParenSummary summary =
            table.prefix[(1U << remaining) | bits];

        const int consumed =
            std::min(balance,
                     static_cast<int>(summary.closedCount));

        const unsigned source =
            (1U << consumed) - 1;

        const unsigned used =
            _pdep_u32(source, summary.farClosed);

        unmatchedPacked |=
            static_cast<std::uint64_t>(
                summary.farClosed ^ used)
            << offset;

        balance =
            summary.farOpen +
            std::max(0, balance - summary.closedCount);
    }

    const std::uint64_t unmatched =
        _pdep_u64(unmatchedPacked, symbols);

    return equality | (close & ~unmatched);
}

inline Row matchAscendingPacked(
    const Row& openInput,
    const Row& closeInput,
    int bitCount) {
    Row matched{};
    const int words = (bitCount + 63) / 64;
    int balance = 0;

    for (int word = 0; word < words; ++word) {
        matched[word] =
            matchPackedWord(
                openInput[word],
                closeInput[word],
                balance);
    }

    clearOutside(matched, bitCount);
    return matched;
}

inline Row matchDescendingPacked(
    const Row& openInput,
    const Row& closeInput,
    int bitCount) {
    Row matched{};
    const int words = (bitCount + 63) / 64;
    int balance = 0;

    for (int word = words - 1; word >= 0; --word) {
        const std::uint64_t open =
            reverseBits(openInput[word]);
        const std::uint64_t close =
            reverseBits(closeInput[word]);

        matched[word] =
            reverseBits(
                matchPackedWord(open, close, balance));
    }

    clearOutside(matched, bitCount);
    return matched;
}

inline std::uint16_t compressEven(std::uint32_t x) {
    x &= 0x55555555U;
    x = (x | (x >> 1)) & 0x33333333U;
    x = (x | (x >> 2)) & 0x0F0F0F0FU;
    x = (x | (x >> 4)) & 0x00FF00FFU;
    x = (x | (x >> 8)) & 0x0000FFFFU;
    return static_cast<std::uint16_t>(x);
}

inline void resolveRunWord(
    int begin,
    int end,
    int countY,
    const std::uint8_t* freeBits,
    std::uint8_t* newX,
    std::uint8_t* newY) {
    const auto* freeWord =
        reinterpret_cast<const std::uint64_t*>(freeBits);
    auto* xWord =
        reinterpret_cast<std::uint64_t*>(newX);
    auto* yWord =
        reinterpret_cast<std::uint64_t*>(newY);

    for (int position = begin; position < end;) {
        const int word = position >> 6;
        const int wordEnd =
            std::min(end, (word + 1) * 64);

        std::uint64_t mask = freeWord[word];
        mask &= ~std::uint64_t{0} << (position & 63);

        if (wordEnd & 63) {
            mask &=
                (std::uint64_t{1} << (wordEnd & 63)) - 1;
        }

        const int available =
            __builtin_popcountll(mask);
        const int take =
            std::min(countY, available);

        const std::uint64_t source =
            take == 64
                ? ~std::uint64_t{0}
                : (std::uint64_t{1} << take) - 1;

        const std::uint64_t chosen =
            _pdep_u64(source, mask);

        xWord[word] |= chosen;
        yWord[word] |= mask ^ chosen;

        countY -= take;
        position = wordEnd;
    }

    if (countY != 0) {
        std::abort();
    }
}

inline void addRowToHeight(
    std::uint16_t* height,
    const Row& row,
    int columns,
    int delta) {
    const __m256i zero = _mm256_setzero_si256();
    const __m256i one = _mm256_set1_epi16(1);

    const __m256i signs = _mm256_set_epi16(
        static_cast<short>(0x8000),
        0x4000,
        0x2000,
        0x1000,
        0x0800,
        0x0400,
        0x0200,
        0x0100,
        0x0080,
        0x0040,
        0x0020,
        0x0010,
        0x0008,
        0x0004,
        0x0002,
        0x0001);

    for (int offset = 0; offset < columns; offset += 16) {
        const int word = offset >> 6;
        const int shift = offset & 63;

        std::uint16_t mask =
            static_cast<std::uint16_t>(
                row[word] >> shift);

        if (shift > 48 && word + 1 < kWords) {
            mask |= static_cast<std::uint16_t>(
                row[word + 1] << (64 - shift));
        }

        const __m256i broadcast =
            _mm256_set1_epi16(
                static_cast<short>(mask));

        const __m256i selected =
            _mm256_and_si256(broadcast, signs);

        __m256i bits =
            _mm256_andnot_si256(
                _mm256_cmpeq_epi16(selected, zero),
                one);

        if (delta < 0) {
            bits = _mm256_sub_epi16(zero, bits);
        }

        const __m256i old =
            _mm256_loadu_si256(
                reinterpret_cast<const __m256i*>(
                    height + offset));

        _mm256_storeu_si256(
            reinterpret_cast<__m256i*>(height + offset),
            _mm256_add_epi16(old, bits));
    }
}

inline void benderKnuth(
    Row& x,
    Row& y,
    std::uint16_t* height,
    int columns,
    bool updateHeight) {
    Row freeRow;
    Row newX;
    Row newY;

    const int words = (columns + 63) / 64;

    for (int word = 0; word < words; ++word) {
        const std::uint64_t paired =
            x[word] & y[word];

        freeRow[word] = x[word] ^ y[word];
        newX[word] = paired;
        newY[word] = paired;
    }

    std::array<std::uint8_t, kBits / 8 + 2> boundary;

    for (int offset = 0; offset < columns; offset += 16) {
        const __m256i current =
            _mm256_loadu_si256(
                reinterpret_cast<const __m256i*>(
                    height + offset));

        const __m256i previous =
            _mm256_loadu_si256(
                reinterpret_cast<const __m256i*>(
                    height + offset - 1));

        const std::uint32_t equal =
            static_cast<std::uint32_t>(
                _mm256_movemask_epi8(
                    _mm256_cmpeq_epi16(
                        current, previous)));

        const std::uint16_t starts =
            compressEven(~equal);

        boundary[offset / 8] =
            static_cast<std::uint8_t>(starts & 255);

        boundary[offset / 8 + 1] =
            static_cast<std::uint8_t>(starts >> 8);
    }

    const auto* freeBytes =
        reinterpret_cast<const std::uint8_t*>(
            freeRow.data());

    const auto* oldY =
        reinterpret_cast<const std::uint8_t*>(
            y.data());

    auto* outX =
        reinterpret_cast<std::uint8_t*>(
            newX.data());

    auto* outY =
        reinterpret_cast<std::uint8_t*>(
            newY.data());

    const int bytes = (columns + 7) / 8;
    int carryBegin = 0;
    int carryY = 0;
    Tables& t = tables();

    for (int byte = 0; byte < bytes; ++byte) {
        int validMask = 255;

        if (byte + 1 == bytes && columns % 8) {
            validMask =
                (1 << (columns % 8)) - 1;
        }

        const int free =
            freeBytes[byte] & validMask;
        const int yByte =
            oldY[byte] & free;
        const int starts =
            boundary[byte] & validMask;

        if (!starts) {
            carryY += __builtin_popcount(yByte);
            continue;
        }

        const int first = __builtin_ctz(starts);
        const int last =
            31 - __builtin_clz(starts);

        const int prefixMask =
            (1 << first) - 1;

        carryY +=
            __builtin_popcount(yByte & prefixMask);

        resolveRunWord(
            carryBegin,
            byte * 8 + first,
            carryY,
            freeBytes,
            outX,
            outY);

        const MidEntry entry =
            t.middle[
                t.ternary[free][yByte] * 256 +
                starts];

        outX[byte] |= entry.x;
        outY[byte] |= entry.covered ^ entry.x;

        carryBegin = byte * 8 + last;

        const int suffixMask =
            validMask & ~((1 << last) - 1);

        carryY =
            __builtin_popcount(yByte & suffixMask);
    }

    resolveRunWord(
        carryBegin,
        columns,
        carryY,
        freeBytes,
        outX,
        outY);

    for (int word = 0; word < words; ++word) {
        x[word] = newX[word];
        y[word] = newY[word];
    }

    if (updateHeight) {
        addRowToHeight(
            height, newX, columns, +1);
    }
}

inline int highestBit(
    const Row& row,
    int bitCount) {
    const int words = (bitCount + 63) / 64;

    for (int word = words - 1; word >= 0; --word) {
        std::uint64_t value = row[word];

        if (word + 1 == words && (bitCount & 63)) {
            value &=
                (std::uint64_t{1} << (bitCount & 63)) -
                1;
        }

        if (value) {
            return word * 64 +
                   63 -
                   __builtin_clzll(value);
        }
    }

    return -1;
}

inline void transpose64(std::uint64_t* x) {
    std::uint64_t mask =
        0x00000000FFFFFFFFULL;

    for (int shift = 32; shift != 0;) {
        for (int k = 0;
             k < 64;
             k = (k + shift + 1) & ~shift) {
            const std::uint64_t delta =
                ((x[k] >> shift) ^
                 x[k + shift]) &
                mask;

            x[k] ^= delta << shift;
            x[k + shift] ^= delta;
        }

        shift >>= 1;

        if (shift) {
            mask ^= mask << shift;
        }
    }
}

inline void transposePrefix(
    Matrix& matrix,
    int bitCount) {
    const int blocks =
        (bitCount + 63) / 64;

    alignas(64) std::uint64_t tile[64];
    alignas(64) std::uint64_t other[64];

    for (int br = 0; br < blocks; ++br) {
        for (int bc = br; bc < blocks; ++bc) {
            if (br == bc) {
                for (int row = 0; row < 64; ++row) {
                    tile[row] =
                        matrix[br * 64 + row][bc];
                }

                transpose64(tile);

                for (int row = 0; row < 64; ++row) {
                    matrix[br * 64 + row][bc] =
                        tile[row];
                }
            } else {
                for (int row = 0; row < 64; ++row) {
                    tile[row] =
                        matrix[br * 64 + row][bc];

                    other[row] =
                        matrix[bc * 64 + row][br];
                }

                transpose64(tile);
                transpose64(other);

                for (int row = 0; row < 64; ++row) {
                    matrix[br * 64 + row][bc] =
                        other[row];

                    matrix[bc * 64 + row][br] =
                        tile[row];
                }
            }
        }
    }
}

inline Row batchDelete(
    Matrix& columns,
    int q) {
    const int bitCount = q;
    const int words =
        (bitCount + 63) / 64;

    Row outgoing{};

    for (int c = q - 2; c >= 0; --c) {
        Row& column = columns[c];

        Row incoming =
            matchDescendingPacked(
                outgoing, column, bitCount);

        Row available{};

        for (int word = 0; word < words; ++word) {
            available[word] =
                column[word] & ~incoming[word];
        }

        int height = 0;

        for (int word = 0; word < words; ++word) {
            height +=
                __builtin_popcountll(column[word]);
        }

        if (height > q - 2 - c) {
            const int value =
                highestBit(available, bitCount);

            if (value < 0) {
                std::abort();
            }

            available[value >> 6] ^=
                std::uint64_t{1} <<
                (value & 63);

            incoming[value >> 6] |=
                std::uint64_t{1} <<
                (value & 63);
        }

        for (int word = 0; word < words; ++word) {
            column[word] =
                outgoing[word] | available[word];
        }

        clearOutside(column, q - 1);
        outgoing = incoming;
    }

    clearOutside(outgoing, q - 1);
    return outgoing;
}

inline Row batchInsert(
    Matrix& columns,
    Row incoming,
    int q) {
    const int words = (q + 63) / 64;

    for (int c = 0; c < q - 1; ++c) {
        Row& column = columns[c];

        const Row matched =
            matchAscendingPacked(
                incoming, column, q);

        for (int word = 0; word < words; ++word) {
            column[word] =
                incoming[word] |
                (column[word] & ~matched[word]);
        }

        clearOutside(column, q);
        incoming = matched;
    }

    return incoming;
}

inline std::unique_ptr<Matrix> cutsToMatrix(
    const std::vector<std::uint16_t>& opt,
    int n) {
    const int m = n - 1;
    auto matrix = std::make_unique<Matrix>();
    std::vector<int> rowLength(m);

    for (int q = 1; q <= m; ++q) {
        const int d = m - q + 1;

        for (int row = 0; row < q; ++row) {
            const int l = q - 1 - row;

            const int newLength =
                opt[
                    static_cast<std::size_t>(l) * n +
                    l + d];

            for (int column = rowLength[row];
                 column < newLength;
                 ++column) {
                setBit((*matrix)[q - 1], column);
            }

            rowLength[row] = newLength;
        }
    }

    return matrix;
}

inline std::string encodeCuts(
    const std::vector<std::uint16_t>& opt,
    int n) {
    (void)tables();

    const int m = n - 1;
    auto matrix = cutsToMatrix(opt, n);

    std::string result;
    result.reserve(
        static_cast<std::size_t>(m) *
        (m - 1) / 2);

    std::array<std::uint16_t, kBits + 17>
        heightStorage{};

    std::uint16_t* const height =
        heightStorage.data() + 1;

    for (int q = m; q >= 2; --q) {
        std::fill(height, height + q - 1, 0);
        height[-1] = 0xFFFF;

        for (int value = 0;
             value + 1 < q;
             ++value) {
            benderKnuth(
                (*matrix)[value],
                (*matrix)[value + 1],
                height,
                q - 1,
                true);
        }

        (*matrix)[q - 1].fill(0);

        transposePrefix(*matrix, q);
        const Row removed =
            batchDelete(*matrix, q);
        transposePrefix(*matrix, q);

        for (int value = 0;
             value < q - 1;
             ++value) {
            (*matrix)[value][(q - 1) >> 6] &=
                ~(std::uint64_t{1} <<
                  ((q - 1) & 63));

            clearOutside(
                (*matrix)[value], q - 1);
        }

        (*matrix)[q - 1].fill(0);

        for (int value = 0;
             value < q - 1;
             ++value) {
            result.push_back(
                bit(removed, value) ? '1' : '0');
        }
    }

    return result;
}

inline std::vector<int> matrixToCuts(
    const Matrix& matrix,
    int n) {
    const int m = n - 1;

    std::vector<std::uint16_t> cell(
        static_cast<std::size_t>(m) * m);

    std::vector<int> columnHeight(m);

    for (int value = 0; value < m; ++value) {
        for (int word = 0;
             word < (m + 63) / 64;
             ++word) {
            std::uint64_t positions =
                matrix[value][word];

            while (positions) {
                const int one =
                    __builtin_ctzll(positions);

                const int column =
                    word * 64 + one;

                positions &= positions - 1;

                if (column >= m) {
                    continue;
                }

                const int row =
                    columnHeight[column]++;

                cell[
                    static_cast<std::size_t>(row) *
                        m +
                    column] =
                    static_cast<std::uint16_t>(
                        value + 1);
            }
        }
    }

    std::vector<int> rowLength(m);

    for (int column = 0; column < m; ++column) {
        for (int row = 0;
             row < columnHeight[column];
             ++row) {
            rowLength[row] =
                std::max(
                    rowLength[row],
                    column + 1);
        }
    }

    std::vector<int> cuts(
        static_cast<std::size_t>(n) * n);

    for (int i = 0; i < n; ++i) {
        cuts[
            static_cast<std::size_t>(i) * n +
            i] = i;
    }

    for (int q = 1; q <= m; ++q) {
        const int d = m - q + 1;

        for (int row = 0; row < q; ++row) {
            int low = 0;
            int high = rowLength[row];

            while (low < high) {
                const int middle =
                    (low + high) / 2;

                if (cell[
                        static_cast<std::size_t>(row) *
                            m +
                        middle] <= q) {
                    low = middle + 1;
                } else {
                    high = middle;
                }
            }

            const int l = q - 1 - row;

            cuts[
                static_cast<std::size_t>(l) * n +
                l + d] = low;
        }
    }

    return cuts;
}

inline std::vector<int> decodeCuts(
    int n,
    const std::string& code) {
    (void)tables();

    const int m = n - 1;
    auto matrix = std::make_unique<Matrix>();

    std::vector<std::size_t> rowStart(m + 1);
    std::size_t position = 0;

    for (int i = 1; i < m; ++i) {
        rowStart[i] = position;
        position += m - i;
    }

    std::array<std::uint16_t, kBits + 17>
        heightStorage{};

    std::uint16_t* const height =
        heightStorage.data() + 1;

    for (int q = 2; q <= m; ++q) {
        const int i = m - q + 1;
        Row incoming{};

        for (int value = 0;
             value < q - 1;
             ++value) {
            if (code[rowStart[i] + value] == '1') {
                setBit(incoming, value);
            }
        }

        transposePrefix(*matrix, q);

        const Row leftover =
            batchInsert(*matrix, incoming, q);

        for (int word = 0;
             word < (q + 63) / 64;
             ++word) {
            if (leftover[word]) {
                std::abort();
            }
        }

        Row missing{};

        for (int c = 0; c < q - 1; ++c) {
            int currentHeight = 0;

            for (int word = 0;
                 word < (q + 63) / 64;
                 ++word) {
                currentHeight +=
                    __builtin_popcountll(
                        (*matrix)[c][word]);
            }

            if (currentHeight < q - 1 - c) {
                setBit(missing, c);
            }
        }

        transposePrefix(*matrix, q);
        (*matrix)[q - 1] = missing;

        height[-1] = 0xFFFF;

        for (int c = 0; c < q - 1; ++c) {
            height[c] =
                static_cast<std::uint16_t>(
                    q - 1 - c -
                    bit((*matrix)[q - 2], c) -
                    bit((*matrix)[q - 1], c));
        }

        for (int value = q - 2;
             value >= 0;
             --value) {
            benderKnuth(
                (*matrix)[value],
                (*matrix)[value + 1],
                height,
                q - 1,
                false);

            if (value > 0) {
                addRowToHeight(
                    height,
                    (*matrix)[value - 1],
                    q - 1,
                    -1);
            }
        }

        for (int value = 0; value < q; ++value) {
            clearOutside(
                (*matrix)[value], q);
        }
    }

    return matrixToCuts(*matrix, n);
}

}  // namespace fastcake

#endif

namespace cake4_fast_link {

std::string encodeCuts(
    const std::vector<std::uint16_t>& opt,
    int n) {
    return fastcake::encodeCuts(opt, n);
}

std::vector<int> decodeCuts(
    int n,
    const std::string& code) {
    return fastcake::decodeCuts(n, code);
}

}  // namespace cake4_fast_link

namespace {

constexpr int kMaxN = 2000;

using WeightTable =
    std::array<
        std::array<long long, kMaxN>,
        kMaxN>;

std::size_t index(int n, int l, int r) {
    return static_cast<std::size_t>(l) * n + r;
}

std::vector<std::uint16_t> computeCuts(
    int n,
    const WeightTable& w) {
    std::vector<__int128> dp(
        static_cast<std::size_t>(n) * n);

    std::vector<std::uint16_t> opt(
        static_cast<std::size_t>(n) * n);

    for (int i = 0; i < n; ++i) {
        opt[index(n, i, i)] =
            static_cast<std::uint16_t>(i);
    }

    for (int length = 2; length <= n; ++length) {
        for (int l = 0;
             l + length <= n;
             ++l) {
            const int r = l + length - 1;

            const int low =
                opt[index(n, l, r - 1)];

            const int high =
                std::min<int>(
                    opt[index(n, l + 1, r)],
                    r - 1);

            __int128 best = -1;
            int bestCut = low;

            for (int k = low; k <= high; ++k) {
                const __int128 candidate =
                    dp[index(n, l, k)] +
                    dp[index(n, k + 1, r)];

                if (best < 0 || candidate < best) {
                    best = candidate;
                    bestCut = k;
                }
            }

            dp[index(n, l, r)] =
                best + w[l][r];

            opt[index(n, l, r)] =
                static_cast<std::uint16_t>(
                    bestCut);
        }
    }

    return opt;
}

std::string encodeUnary(
    const std::vector<std::uint16_t>& opt,
    int n) {
    std::string result;

    result.reserve(
        static_cast<std::size_t>(n - 1) *
        (n - 1));

    for (int l = 0; l + 1 < n; ++l) {
        int previous = l;

        for (int r = l + 1; r < n; ++r) {
            const int current =
                opt[index(n, l, r)];

            result.append(
                current - previous, '1');

            result.push_back('0');
            previous = current;
        }
    }

    return result;
}

}  // namespace

std::string encode(
    int n,
    const std::array<
        std::array<long long, 2000>,
        2000>& w) {
    const std::vector<std::uint16_t> cuts =
        computeCuts(n, w);

    if (n <= 1414) {
        return encodeUnary(cuts, n);
    }

    if (n < 2000) {
        std::string unary =
            encodeUnary(cuts, n);

        if (unary.size() + 1 <= 1997001) {
            unary.insert(unary.begin(), '0');
            return unary;
        }

        std::string tournament =
            cake4_fast_link::encodeCuts(cuts, n);

        tournament.insert(
            tournament.begin(), '1');

        return tournament;
    }

    return cake4_fast_link::encodeCuts(cuts, n);
}

#include "Cake_4.h"

#include <cstddef>
#include <string>
#include <vector>

namespace cake4_fast_link {

std::vector<int> decodeCuts(
    int n,
    const std::string& code);

}  // namespace cake4_fast_link

namespace {

int decodedN;
std::vector<int> decodedCuts;


}  // namespace

void decode(int n, const std::string& s) {
    decodedN = n;

    if (n == 2000) {
        decodedCuts =
            cake4_fast_link::decodeCuts(n, s);
        return;
    }

    if (n > 1414 && s[0] == '1') {
        decodedCuts =
            cake4_fast_link::decodeCuts(
                n, s.substr(1));
        return;
    }

    decodedCuts.assign(
        static_cast<std::size_t>(n) * n, 0);

    for (int i = 0; i < n; ++i) {
        decodedCuts[index(n, i, i)] = i;
    }

    std::size_t position =
        (n > 1414 ? 1 : 0);

    for (int l = 0; l + 1 < n; ++l) {
        int current = l;

        for (int r = l + 1; r < n; ++r) {
            while (s[position] == '1') {
                ++current;
                ++position;
            }

            ++position;
            decodedCuts[index(n, l, r)] =
                current;
        }
    }
}

int query(int l, int r) {
    return decodedCuts[
        index(decodedN, l, r)];
}