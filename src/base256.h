#ifndef VEC_OPERATIONS_H
#define VEC_OPERATIONS_H

#include <algorithm>
#include <cstdint>
#include <iostream>

#include "helper.h"

namespace operations {
class Base256 {
   public:
    Base256(const std::uint64_t initialValue) { data = convertToVector(initialValue); }

    Base256(const Base256 &base256) { data = base256.data; }

    Base256() { data = convertToVector(0); }

    void add(const ByteArray &b) noexcept;

    void sub(const ByteArray &b) noexcept;

    ByteArray sub(const ByteArray &a, const ByteArray &b) noexcept;

    void mul(const ByteArray &b) noexcept;

    void div(const ByteArray &divisor, ByteArray *remaining = nullptr) noexcept;

    static Base256 pow(const Base256 &a, const std::uint64_t &pow);

    void print() const {
        // Make a copy because we will modify it
        ByteArray temp = data;

        // Handle zero explicitly
        if (isZero(temp)) {
            std::cout << "0" << std::endl;
            return;
        }

        std::string result;

        while (!isZero(temp)) {
            std::uint16_t remainder = 0;

            // Divide temp by 10 (base256 -> base10 conversion step)
            for (std::int64_t i = static_cast<std::int64_t>(temp.size()) - 1; i >= 0; --i) {
                const std::uint16_t current = (remainder << 8) | temp[i];
                temp[i] = static_cast<std::uint8_t>(current / 10);
                remainder = current % 10;
            }

            result.push_back(static_cast<char>('0' + remainder));
        }

        std::ranges::reverse(result);
        std::cout << result << std::endl;
    }

    static void normalizeVector(ByteArray &a) {
        while (a.size() > 1 && a.back() == 0) {
            a.pop_back();
        }
    }

    Base256 &operator=(const Base256 &other) {
        // We protect us from self assignment
        if (this != &other) {
            data = other.data;
        }
        return *this;
    }

    friend Base256 operator+(const Base256 &rhs, const Base256 &lhs) {
        Base256 result(rhs);
        result += lhs;
        return result;
    }

    Base256 &operator+=(const Base256 &rhs) {
        add(rhs.data);
        return *this;
    }

    friend Base256 operator-(const Base256 &rhs, const Base256 &lhs) {
        Base256 result(rhs);
        result -= lhs;
        return result;
    }

    Base256 &operator-=(const Base256 &rhs) {
        sub(rhs.data);
        return *this;
    }

    friend Base256 operator*(const Base256 &rhs, const Base256 &lhs) {
        Base256 result(rhs);
        result *= lhs;
        return result;
    }

    Base256 &operator*=(const Base256 &rhs) {
        mul(rhs.data);
        return *this;
    }

    friend Base256 operator/(const Base256 &rhs, const Base256 &lhs) {
        Base256 result(rhs);
        result /= lhs;
        return result;
    }

    Base256 &operator/=(const Base256 &rhs) {
        div(rhs.data);
        return *this;
    }

    friend Base256 operator%(const Base256 &rhs, const Base256 &lhs) {
        Base256 result(rhs);
        result %= lhs;
        return result;
    }

    Base256 &operator%=(const Base256 &rhs) {
        Base256 remaining;
        this->div(rhs.data, &remaining.data);
        this->data = std::move(remaining.data);
        return *this;
    }

    [[nodiscard]] friend bool operator==(const Base256 &lhs, const Base256 &rhs) {
        return isEqual(lhs.data, rhs.data);
    }

    [[nodiscard]] friend bool operator!=(const Base256 &lhs, const Base256 &rhs) {
        return !isEqual(lhs.data, rhs.data);
    }

    [[nodiscard]] friend bool operator>(const Base256 &lhs, const Base256 &rhs) {
        return isBigger(lhs.data, rhs.data);
    }

    [[nodiscard]] friend bool operator<(const Base256 &lhs, const Base256 &rhs) {
        return isBigger(rhs.data, lhs.data);
    }

    [[nodiscard]] friend bool operator>=(const Base256 &lhs, const Base256 &rhs) {
        return !isBigger(rhs.data, lhs.data);
    }

    [[nodiscard]] friend bool operator<=(const Base256 &lhs, const Base256 &rhs) {
        return !isBigger(lhs.data, rhs.data);
    }

   private:
    ByteArray data;
};
}  // namespace operations

#endif
