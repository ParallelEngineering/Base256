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

    // Handle an underflow when subtracting
    bool borrow = false;

    for (int i = 0; i < a.size(); i++) {
        std::int32_t subtract;
        // Check for the end of the subtractor
        if (i >= b.size()) {
            subtract = borrow;
        } else {
            subtract = borrow + b[i];
        }
        borrow = false;

        if (a[i] >= subtract) {
            // If the current number is as least as big as subtract
            std::uint8_t number = a[i] - subtract;
            result.push_back(number);
        } else {
            // Borrow from the next number
            subtract -= 256;
            borrow = true;

            // Here subtract can only be 0 or negative
            std::uint8_t number = a[i] - subtract;
            result.push_back(number);
        }
    }

    // Strip trailing zeroes to normalize
    normalizeVector(result);

    return result;
}

// The return value can only be positive, if it would be negative, 0 is returned
void operations::Base256::sub(const ByteArray &b) noexcept {
    ByteArray result = sub(data, b);
    data = std::move(result);
}

void operations::Base256::subInPlace(ByteArray &a, const ByteArray &b) {
    // Safely clamp to 0 if the number being subtracted is larger than the base
    if (isBigger(b, a)) {
        return;
    }

    // Handle an underflow when subtracting
    bool borrow = false;

    for (int i = 0; i < a.size(); i++) {
        std::int32_t subtract;
        // Check for the end of the subtractor
        if (i >= b.size()) {
            subtract = borrow;
        } else {
            subtract = borrow + b[i];
        }
        borrow = false;

        if (a[i] >= subtract) {
            // If the current number is as least as big as subtract
            std::uint8_t number = a[i] - subtract;
            a[i] = number;
        } else {
            // Borrow from the next number
            subtract -= 256;
            borrow = true;

            // Here subtract can only be 0 or negative
            std::uint8_t number = a[i] - subtract;
            a[i] = number;
        }
    }

    // Strip trailing zeroes to normalize
    normalizeVector(a);
}

void operations::Base256::mul(const ByteArray &b) noexcept {
    const std::uint64_t aSize = data.size();
    const std::uint64_t bSize = b.size();

    // Initialize the result vector with zeros, with the size of aSize + bSize
    ByteArray result(aSize + bSize, 0);

    for (std::uint64_t i = 0; i < aSize; i++) {
        // Set up the carry value for each iteration
        std::uint16_t carry = 0;

        for (uint64_t x = 0; x < bSize; x++) {
            // Calculate the product by adding up the previous result, the carry, and the new
            // product
            const std::uint16_t product = result[i + x] + carry + (data[i] * b[x]);

            // Calculate the carry which is the overflow beyond 255
            carry = product >> 8;

            // Store the least significant byte (lsb) of the product in the result
            result[i + x] = static_cast<std::uint8_t>(product & 0xFF);
        }

        // If there is a carry, append it to the result
        // But with the offset of bSize because of the previous inner loop
        result[i + bSize] += static_cast<std::uint8_t>(carry);
    }

    // Since aSize + bSize typically provides extra buffering, normalize the number safely
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

    // Pre allocate vector
    ByteArray quotient((initialDividendIndex / 8) + 1, 0);

    std::int64_t dividendIndex = initialDividendIndex;
    ByteArray dividendMask = addBitFromNumber({0}, data, dividendIndex--);

    while (dividendIndex >= -1) {
        std::int64_t currentQBitIndex = dividendIndex + 1;

        if (isEqual(dividendMask, divisor) || isBigger(dividendMask, divisor)) {
            // Drop evaluating bit immediately at explicitly targeted position inside quotient
            quotient[currentQBitIndex / 8] |= (1 << (currentQBitIndex % 8));

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

    // Strip trailing normalization zeros (empty space buffers) securely
    normalizeVector(quotient);

    if (quotient.empty()) quotient.push_back(0);

    data = std::move(quotient);
}