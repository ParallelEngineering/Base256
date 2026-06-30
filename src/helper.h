#ifndef BASE_256_HELPER_H
#define BASE_256_HELPER_H

#include <cassert>

#include <cstdint>

#include "types.h"

// This function returns the index of the most significant bit
[[nodiscard]] inline std::int64_t getStartBitIndex(const ByteArray &a) {
    // Handle the absolute zero edge-case
    if (a.empty() || (a.size() == 1 && a[0] == 0)) {
        return INVALID_START_BIT_INDEX;
    }

    // Because of normalization, the highest bit is guaranteed
    // to be in the very last byte of the vector.
    const auto highestByteIndex = static_cast<std::int64_t>(a.size() - 1);
    const std::uint64_t highestByte = a.back();

    // Find the highest bit in just this one byte (max 64 iterations)
    for (std::int64_t bit = 63; bit >= 0; bit--) {
        if ((highestByte & (0b1 << bit)) != 0) {
            // Calculate total bit index: (Byte Position * 8) + Bit Position
            return (highestByteIndex * 64) + bit;
        }
    }

    assert(false);
}

[[nodiscard]] inline ByteArray convertToVector(const std::uint64_t number) noexcept {
    ByteArray result;

    // Ensure 0 results in at least [0], never[]
    if (number == 0) return {0};
    result.push_back(number);
    return result;
}

[[nodiscard]] inline ByteArray addBitFromNumber(const ByteArray &numberToShift,
                                                const ByteArray &sourceNumber,
                                                const std::int64_t bitIndex) {
    ByteArray result;

    // Validated boundary access
    if (bitIndex < 0 || bitIndex > getStartBitIndex(sourceNumber)) {
        return {0};
    }

    // bitIndex & 63 is equivalent to bitIndex % 64
    const int sourceNumberMask = 0b1 << (bitIndex & 63);
    constexpr std::uint64_t mask = 0x7FFF'FFFF'FFFF'FFFFULL;

    const int sourceNumberIndex = bitIndex / 64;

    bool mostSignificantBit = (sourceNumber[sourceNumberIndex] & sourceNumberMask) != 0;

    for (const std::uint64_t currentByte : numberToShift) {
        // The mask for MSB is 0x80 (128). We evaluate this BEFORE currentByte gets
        // overwritten/shifted.
        const bool nextMSB = (currentByte & mask) != 0;
        result.push_back((currentByte | mostSignificantBit));
        mostSignificantBit = nextMSB;
    }

    if (mostSignificantBit) result.push_back(0b1);

    return result;
}

inline void addBitFromNumberInPlace(ByteArray &numberToShift,
                                                const ByteArray &sourceNumber,
                                                const std::int64_t bitIndex) {

    // Validated boundary access
    if (bitIndex < 0 || bitIndex > getStartBitIndex(sourceNumber)) {
        return;
    }

    // bitIndex & 7 is equivalent to bitIndex % 8
    const int sourceNumberMask = 0b1 << (bitIndex & 63);
    constexpr std::uint64_t mask = 0x7FFF'FFFF'FFFF'FFFFULL;

    // bitIndex >> 3 is equivalent to bitIndex / 8
    const int sourceNumberIndex = bitIndex / 64;

    bool mostSignificantBit = (sourceNumber[sourceNumberIndex] & sourceNumberMask) != 0;
    for (std::uint64_t i : numberToShift) {
        // The mask for MSB is 0x80 (128). We evaluate this BEFORE currentByte gets
        // overwritten/shifted.
        const std::uint8_t currentByte = i;
        const bool nextMSB = (currentByte & mask) != 0;
        i = currentByte | mostSignificantBit;
        mostSignificantBit = nextMSB;
    }

   if (mostSignificantBit) numberToShift.push_back(0b1);
}

[[nodiscard]] inline bool isBigger(const ByteArray &a, const ByteArray &b) noexcept {
    const std::uint32_t aSize = a.size();
    const std::uint32_t bSize = b.size();
    const std::uint64_t iterations = aSize > bSize ? aSize : bSize;

    for (std::int64_t i = iterations - 1; i >= 0; i--) {
        const std::uint8_t aValue = (i < aSize) ? a[i] : 0;
        const std::uint8_t bValue = (i < bSize) ? b[i] : 0;

        if (aValue > bValue) {
            return true;
        }

        // Explicitly return false if the checking bit drops behind
        if (aValue < bValue) {
            return false;
        }
    }

    return false;
}

[[nodiscard]] inline bool isEqual(const ByteArray &a, const ByteArray &b) noexcept {
    const std::uint32_t aSize = a.size();
    const std::uint32_t bSize = b.size();
    const std::uint64_t iterations = aSize > bSize ? aSize : bSize;

    const std::uint32_t shortest = aSize < bSize ? aSize : bSize;
    const ByteArray longest = aSize > bSize ? a : b;

    for (std::uint64_t i = 0; i < iterations; i++) {
        if (i >= shortest) {
            if (longest[i] != 0) return false;
        } else {
            if (a[i] != b[i]) return false;
        }
    }

    return true;
}

[[nodiscard]] inline bool isZero(const ByteArray &a) {
    for (const std::uint64_t number : a) {
        if (number != 0) return false;
    }
    return true;
}

#endif  // BASE_256_HELPER_H
