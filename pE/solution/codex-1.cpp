#include "Cake_4.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#pragma GCC optimize("O3")
#pragma GCC target("avx2,popcnt,lzcnt,abm,bmi,bmi2,tune=native") // avx or sse


namespace {
using Matrix = std::array<std::array<long long, 2000>, 2000>;

int decodedN;
std::vector<int> decodedCuts;

std::size_t index(int n, int l, int r) {
    return static_cast<std::size_t>(l) * n + r;
}

class BigUIntDecode {
public:
    explicit BigUIntDecode(std::uint32_t value = 0) : limb_(1, value) {}

    void multiply(std::uint32_t multiplier) {
        std::uint64_t carry = 0;
        for (std::uint32_t& x : limb_) {
            const std::uint64_t value =
                static_cast<std::uint64_t>(x) * multiplier + carry;
            x = static_cast<std::uint32_t>(value);
            carry = value >> 32;
        }
        if (carry != 0) limb_.push_back(static_cast<std::uint32_t>(carry));
    }

    int encodedBits() const {
        if (limb_.size() == 1 && limb_[0] == 1) return 0;

        int nonzero = 0;
        bool powerOfTwo = true;
        for (std::uint32_t x : limb_) {
            if (x == 0) continue;
            ++nonzero;
            if ((x & (x - 1)) != 0) powerOfTwo = false;
        }
        powerOfTwo = powerOfTwo && nonzero == 1;

        const std::uint32_t top = limb_.back();
        const int bitLength = static_cast<int>((limb_.size() - 1) * 32) +
                              32 - __builtin_clz(top);
        return bitLength - (powerOfTwo ? 1 : 0);
    }

    void setBit(int position) {
        const std::size_t word = static_cast<std::size_t>(position) / 32;
        if (limb_.size() <= word) limb_.resize(word + 1);
        limb_[word] |= std::uint32_t{1} << (position & 31);
    }

    std::uint32_t divide(std::uint32_t divisor) {
        std::uint64_t remainder = 0;
        for (std::size_t i = limb_.size(); i-- > 0;) {
            const std::uint64_t value = (remainder << 32) | limb_[i];
            limb_[i] = static_cast<std::uint32_t>(value / divisor);
            remainder = value % divisor;
        }
        while (limb_.size() > 1 && limb_.back() == 0) limb_.pop_back();
        return static_cast<std::uint32_t>(remainder);
    }

private:
    std::vector<std::uint32_t> limb_;
};

}  // namespace

void decode(int n, const std::string& s) {
    decodedN = n;
    decodedCuts.assign(static_cast<std::size_t>(n) * n, 0);

    std::vector<int> previous(n);
    for (int l = 0; l < n; ++l) {
        previous[l] = l;
        decodedCuts[index(n, l, l)] = l;
    }

    std::size_t position = 0;
    for (int d = 1; d < n; ++d) {
        const int length = n - d;
        std::vector<int> current(length);
        std::vector<int> radix(length);
        BigUIntDecode possibilities(1);

        for (int l = 0; l < length; ++l) {
            const int lo = previous[l];
            const int hi = previous[l + 1];
            radix[l] = hi - lo + 1;
            possibilities.multiply(static_cast<std::uint32_t>(radix[l]));
        }

        const int bits = possibilities.encodedBits();
        BigUIntDecode rank;
        for (int b = 0; b < bits; ++b) {
            if (s[position++] == '1') rank.setBit(b);
        }

        for (int l = length - 1; l >= 0; --l) {
            const int digit =
                static_cast<int>(rank.divide(static_cast<std::uint32_t>(radix[l])));
            current[l] = previous[l] + digit;
            decodedCuts[index(n, l, l + d)] = current[l];
        }

        previous.swap(current);
    }
}

int query(int l, int r) {
    return decodedCuts[index(decodedN, l, r)];
}

class BigUInt {
public:
    explicit BigUInt(std::uint32_t value = 0) : limb_(1, value) {}

    void multiplyAdd(std::uint32_t multiplier, std::uint32_t addend) {
        std::uint64_t carry = addend;
        for (std::uint32_t& x : limb_) {
            const std::uint64_t value =
                static_cast<std::uint64_t>(x) * multiplier + carry;
            x = static_cast<std::uint32_t>(value);
            carry = value >> 32;
        }
        if (carry != 0) limb_.push_back(static_cast<std::uint32_t>(carry));
    }

    int encodedBits() const {
        if (limb_.size() == 1 && limb_[0] == 1) return 0;

        int nonzero = 0;
        bool powerOfTwo = true;
        for (std::uint32_t x : limb_) {
            if (x == 0) continue;
            ++nonzero;
            if ((x & (x - 1)) != 0) powerOfTwo = false;
        }
        powerOfTwo = powerOfTwo && nonzero == 1;

        const std::uint32_t top = limb_.back();
        const int bitLength = static_cast<int>((limb_.size() - 1) * 32) +
                              32 - __builtin_clz(top);
        return bitLength - (powerOfTwo ? 1 : 0);
    }

    bool bit(int position) const {
        const std::size_t word = static_cast<std::size_t>(position) / 32;
        return word < limb_.size() &&
               ((limb_[word] >> (position & 31)) & 1U) != 0;
    }

private:
    std::vector<std::uint32_t> limb_;
};

std::vector<std::uint16_t> computeCuts(
    const int n, const Matrix& w) {
    std::vector<__int128> dp(static_cast<std::size_t>(n) * n);
    std::vector<std::uint16_t> opt(static_cast<std::size_t>(n) * n);

    for (int i = 0; i < n; ++i) {
        opt[index(n, i, i)] = static_cast<std::uint16_t>(i);
    }

    for (int length = 2; length <= n; ++length) {
        for (int l = 0; l + length <= n; ++l) {
            const int r = l + length - 1;
            const int low = opt[index(n, l, r - 1)];
            const int high = std::min<int>(opt[index(n, l + 1, r)], r - 1);

            __int128 best = -1;
            int bestCut = low;
            for (int k = low; k <= high; ++k) {
                const __int128 candidate =
                    dp[index(n, l, k)] + dp[index(n, k + 1, r)];
                if (best < 0 || candidate < best) {
                    best = candidate;
                    bestCut = k;
                }
            }

            dp[index(n, l, r)] = best + w[l][r];
            opt[index(n, l, r)] = static_cast<std::uint16_t>(bestCut);
        }
    }
    return opt;
}

std::string encode(int n, const Matrix& w) {
    const std::vector<std::uint16_t> opt = computeCuts(n, w);

    std::string result;
    result.reserve(static_cast<std::size_t>(n) * n * 3 / 4);

    // previous[l] is K_{d-1}(l).  K_0(l) = l.
    std::vector<int> previous(n);
    for (int l = 0; l < n; ++l) previous[l] = l;

    for (int d = 1; d < n; ++d) {
        const int length = n - d;
        std::vector<int> current(length);
        BigUInt rank;
        BigUInt possibilities(1);

        for (int l = 0; l < length; ++l) {
            const int lo = previous[l];
            const int hi = previous[l + 1];
            const int radix = hi - lo + 1;
            const int digit = opt[index(n, l, l + d)] - lo;

            rank.multiplyAdd(static_cast<std::uint32_t>(radix),
                             static_cast<std::uint32_t>(digit));
            possibilities.multiplyAdd(static_cast<std::uint32_t>(radix), 0);
            current[l] = digit + lo;
        }

        const int bits = possibilities.encodedBits();
        for (int b = 0; b < bits; ++b) {
            result.push_back(rank.bit(b) ? '1' : '0');
        }

        previous.swap(current);
    }

    return result;
}
