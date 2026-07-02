#include "base256.h"

#include <algorithm>

void operations::Base256::add(const ByteArray &b) noexcept {
    ByteArray result;

    // Get the max iterations based on the largest vector
    const std::size_t iterations = std::max(data.size(), b.size());
    result.reserve(iterations + 1);

    // Carry to handle the overflow. This can either be 0 or 1
    std::uint64_t carry = 0;

    for (std::size_t i = 0; i < iterations; ++i) {
        const std::uint64_t aVal = (i < data.size()) ? data[i] : 0ULL;
        const std::uint64_t bVal = (i < b.size()) ? b[i] : 0ULL;

        // Add both values
        std::uint64_t sum = aVal + bVal;

        // If the sum overflows, we can detect this by checking if the sum is smaller than aVal
        // Theoretically we could also use bVal for checking because it is guaranteed that in that case,
        // bot aVal and bVal are greater than sum.
        std::uint64_t nextCarry = (sum < aVal) ? 1ULL : 0ULL;

        // We add our previous carry
        sum += carry;
        if (sum < carry) {
            nextCarry += 1ULL;
        }

        result.push_back(sum);
        carry = nextCarry;
    }

    // If we still have a carry left we need to append it
    if (carry) {
        result.push_back(carry);
    }

    normalizeVector(result);

    data = std::move(result);
}

[[nodiscard]] ByteArray operations::Base256::sub(const ByteArray &a, const ByteArray &b) noexcept {
    // Safely clamp to 0 if the number being subtracted is larger than the base
    if (isBigger(b, a)) {
        return {0};
    }

    ByteArray result;
    result.reserve(a.size());

    // Track the borrow state across the subtraction steps
    bool borrow = false;

    for (std::size_t i = 0; i < a.size(); ++i) {
        const std::uint64_t aVal = a[i];
        const std::uint64_t bVal = (i < b.size()) ? b[i] : 0ULL;

        // Subtract bVal from aVal and check for an underflow (borrow)
        std::uint64_t diff = aVal - bVal;
        bool nextBorrow = (aVal < bVal);

        // Subtract the previous borrow and check for an additional underflow
        const std::uint64_t borrowVal = borrow ? 1ULL : 0ULL;
        std::uint64_t finalDiff = diff - borrowVal;
        if (diff < borrowVal) {
            nextBorrow = true;
        }

        result.push_back(finalDiff);
        borrow = nextBorrow;
    }

    // Strip trailing zeroes to normalize the result
    normalizeVector(result);

    return result;
}

void operations::Base256::subInPlace(ByteArray &a, const ByteArray &b) {
    // Safely clamp to 0 if the number being subtracted is larger than the base
    if (isBigger(b, a)) {
        a = {0};
        return;
    }

    // Track the borrow state across the subtraction steps
    bool borrow = false;

    for (std::size_t i = 0; i < a.size(); ++i) {
        const std::uint64_t aVal = a[i];
        const std::uint64_t bVal = (i < b.size()) ? b[i] : 0ULL;

        // Subtract bVal from aVal and check for an underflow (borrow)
        std::uint64_t diff = aVal - bVal;
        bool nextBorrow = (aVal < bVal);

        // Subtract the previous borrow and check for an additional underflow
        const std::uint64_t borrowVal = borrow ? 1ULL : 0ULL;
        std::uint64_t finalDiff = diff - borrowVal;
        if (diff < borrowVal) {
            nextBorrow = true;
        }

        a[i] = finalDiff;
        borrow = nextBorrow;
    }

    // Strip trailing zeroes to normalize the result
    normalizeVector(a);
}

// The return value can only be positive, if it would be negative, 0 is returned
void operations::Base256::sub(const ByteArray &b) noexcept {
    ByteArray result = sub(data, b);
    data = std::move(result);
}

void operations::Base256::mul(const ByteArray &b) noexcept {
    if (data.empty() || b.empty()) {
        data.clear();
        return;
    }

    size_t size_a = data.size();
    size_t size_b = b.size();

    ByteArray result(size_a + size_b, 0);

    for (size_t i = 0; i < size_a; ++i) {
        std::uint64_t carry = 0;
        std::uint64_t valA = data[i];

        for (size_t j = 0; j < size_b; ++j) {
            std::uint64_t valB = b[j];

            std::uint64_t n1 = valA >> 32;
            std::uint64_t n2 = valA & 0xFFFFFFFFULL;

            std::uint64_t n3 = valB >> 32;
            std::uint64_t n4 = valB & 0xFFFFFFFFULL;

            std::uint64_t r1 = n1 * n3;
            std::uint64_t r2 = n2 * n4;
            std::uint64_t r3 = n1 * n4;
            std::uint64_t r4 = n2 * n3;

            std::uint64_t c1 = r1 >> 32;
            std::uint64_t c2 = r3 >> 32;
            std::uint64_t fr1 = (r1 << 32) | (r3 & 0xFFFFFFFFULL);
            std::uint64_t fc1 = (c1 << 32) | c2;

            std::uint64_t c3 = r2 >> 32;
            r2 &= 0xFFFFFFFFULL;
            std::uint64_t c4 = r4 >> 32;
            std::uint64_t fr2 = (r4 << 32) | r2;
            std::uint64_t fc2 = c3 | (c4 << 32);

            std::uint64_t fl = fr2;
            std::uint64_t runningCarry = 0;
            std::uint64_t valFL = fc2 << 32;
            fl += valFL;
            if (valFL > fl) {
                runningCarry += 1ULL;
            }

            std::uint64_t valFH = fr1 << 32;
            fl += valFH;
            if (valFH > fl) {
                runningCarry += 1ULL;
            }

            std::uint64_t fh = (fc2 >> 32) + (fr1 >> 32) + fc1 + runningCarry;


            std::uint64_t current = result[i + j];

            std::uint64_t sum_low = current + fl;
            std::uint64_t carry_out1 = (sum_low < current) ? 1ULL : 0ULL;

            std::uint64_t sum_low_prev = sum_low;
            sum_low += carry;
            std::uint64_t carry_out2 = (sum_low < sum_low_prev) ? 1ULL : 0ULL;

            result[i + j] = sum_low;

            carry = fh + carry_out1 + carry_out2;
        }
        result[i + size_b] = carry;
    }

    normalizeVector(result);

    data = std::move(result);
}

void operations::Base256::div(const ByteArray &divisor, ByteArray *remaining) noexcept {
    const std::int64_t initialDividendIndex = getStartBitIndex(data);
    if (isZero(divisor) || initialDividendIndex == INVALID_START_BIT_INDEX) {
        if (remaining != nullptr) *remaining = {0};
        data = {0};
        return;
    }

    ByteArray quotient((initialDividendIndex / 64) + 1, 0);

    std::int64_t dividendIndex = initialDividendIndex;
    ByteArray dividendMask = addBitFromNumber({0}, data, dividendIndex--);

    while (dividendIndex >= -1) {
        std::int64_t currentQBitIndex = dividendIndex + 1;

        if (isEqual(dividendMask, divisor) || isBigger(dividendMask, divisor)) {
            quotient[currentQBitIndex / 64] |= (1ULL << (currentQBitIndex % 64));

            if (dividendIndex < 0) {
                if (remaining != nullptr) *remaining = sub(dividendMask, divisor);
                break;
            }

            subInPlace(dividendMask, divisor);
            addBitFromNumberInPlace(dividendMask, data, dividendIndex);
            dividendIndex--;
        } else {
            if (dividendIndex < 0) {
                if (remaining != nullptr) *remaining = dividendMask;
                break;
            }

            addBitFromNumberInPlace(dividendMask, data, dividendIndex);
            dividendIndex--;
        }
    }

    // Remove trailing zeros
    normalizeVector(quotient);

    if (quotient.empty()) quotient.push_back(0);

    data = std::move(quotient);
}