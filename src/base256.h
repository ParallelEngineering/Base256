#ifndef VEC_OPERATIONS_H
#define VEC_OPERATIONS_H

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <utility>

#include "helper.h"

namespace operations {
    class Base256 {
    public:
        Base256(const std::uint64_t initialValue) { data = convertToVector(initialValue); }

        Base256(const Base256 &base256) { data = base256.data; }

        Base256(ByteArray bytes) : data(std::move(bytes)) { }

        Base256() { data = convertToVector(0); }

        [[nodiscard]] const ByteArray &getBytes() const { return data; }

        void add(const ByteArray &b) noexcept;

        void sub(const ByteArray &b) noexcept;

        void subInPlace(ByteArray &a, const ByteArray &b);

        ByteArray sub(const ByteArray &a, const ByteArray &b) noexcept;

        void mul(const ByteArray &b) noexcept;

        void div(const ByteArray &divisor, ByteArray *remaining = nullptr) noexcept;

        void print() const {
            ByteArray temp = data;

            if (isZero(temp)) {
                std::cout << "0" << std::endl;
                return;
            }

            std::string result;

            while (!isZero(temp)) {
                std::uint64_t remainder = 0;

                // TODO protable fallback
                for (std::int64_t i = static_cast<std::int64_t>(temp.size()) - 1; i >= 0; --i) {
                    const __uint128_t current = (static_cast<__uint128_t>(remainder) << 64) | temp[i];
                    temp[i] = static_cast<std::uint64_t>(current / 10);
                    remainder = static_cast<std::uint64_t>(current % 10);
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
} // namespace operations

#endif
