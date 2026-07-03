#include "math_utils.h"
#include <vector>
#include "helper.h"

namespace operations::math {
    namespace {
        // Helper function to shift a Base256 number right by n bytes (dividing by 256^n)
        Base256 shiftRightBytes(const Base256 &val, size_t n) {
            const auto &bytes = val.getBytes();
            if (n >= bytes.size()) {
                return Base256(0);
            }
            ByteArray newBytes(bytes.begin() + n, bytes.end());
            return Base256(newBytes);
        }

        // Barrett Reducer encapsulation to replace slow divisions
        struct BarrettReducer {
            Base256 modulus;
            size_t k;
            Base256 mu;

            BarrettReducer(const Base256 &mod) : modulus(mod) {
                k = modulus.getBytes().size();

                // Create 256^(2k) as [0, 0, ..., 1] in little-endian representation
                ByteArray tempBytes(2 * k + 1, 0);
                tempBytes.back() = 1;
                Base256 bigPower(tempBytes);

                // Compute mu = floor(256^(2k) / modulus)
                // This is the only division performed during the entire modPow operation
                mu = bigPower / modulus;
            }

            // Reduces x modulo modulus, assuming x < modulus^2
            Base256 reduce(const Base256 &x) const {
                if (x < modulus) {
                    return x;
                }

                // q1 = x >> (k-1)
                size_t shift1 = (k > 1) ? (k - 1) : 0;
                Base256 q1 = shiftRightBytes(x, shift1);

                // q2 = q1 * mu
                Base256 q2 = q1 * mu;

                // q3 = q2 >> (k+1)
                Base256 q3 = shiftRightBytes(q2, k + 1);

                // r = x - q3 * modulus
                Base256 r = x - (q3 * modulus);

                // Since q3 is an approximation, we adjust at most twice
                while (r >= modulus) {
                    r -= modulus;
                }
                return r;
            }
        };

        Base256 modPowWithReducer(Base256 base, Base256 exponent, const BarrettReducer &reducer) {
            Base256 result(1);
            base = reducer.reduce(base);

            while (!isZero(exponent.getBytes())) {
                if (isOdd(exponent)) {
                    result = reducer.reduce(result * base);
                }
                base = reducer.reduce(base * base);
                exponent = divideByTwo(exponent);
            }
            return result;
        }
    }

    bool isOdd(const Base256 &val) {
        const auto &bytes = val.getBytes();
        if (bytes.empty()) return false;
        return (bytes[0] & 1) != 0;
    }

    Base256 divideByTwo(const Base256 &val) {
        auto bytes = val.getBytes();
        if (bytes.empty()) return Base256(0);

        std::uint64_t carry = 0;

        for (int i = static_cast<int>(bytes.size()) - 1; i >= 0; --i) {
            const std::uint64_t next_carry = (bytes[i] & 1) ? 0x8000'0000'0000'0000ULL : 0;
            bytes[i] = (bytes[i] >> 1) | carry;
            carry = next_carry;
        }

        Base256::normalizeVector(bytes);

        return Base256(bytes);
    }

    Base256 gcd(const Base256 &a, const Base256 &b) {
        Base256 tempA = a;
        Base256 tempB = b;
        while (tempB != Base256(0)) {
            const Base256 t = tempB;
            tempB = tempA % tempB;
            tempA = t;
        }
        return tempA;
    }

    Base256 modInverse(const Base256 &a, const Base256 &m) {
        Base256 r0 = m;
        Base256 r1 = a;
        Base256 x0(0);
        Base256 x1(1);

        bool sign0 = true;
        bool sign1 = true;

        while (r1 != Base256(0)) {
            Base256 q = r0 / r1;
            Base256 r2 = r0 % r1;

            Base256 x2(0);
            bool sign2 = true;

            if (sign0 == sign1) {
                Base256 qx = q * x1;
                if (x0 >= qx) {
                    x2 = x0 - qx;
                    sign2 = sign0;
                } else {
                    x2 = qx - x0;
                    sign2 = !sign0;
                }
            } else {
                x2 = x0 + q * x1;
                sign2 = sign0;
            }

            r0 = r1;
            r1 = r2;
            x0 = x1;
            x1 = x2;
            sign0 = sign1;
            sign1 = sign2;
        }

        if (r0 != Base256(1)) {
            return Base256(0);
        }

        if (!sign0) {
            return m - (x0 % m);
        }
        return x0 % m;
    }

    bool isPrime(const Base256 &n) {
        if (n <= Base256(1)) return false;
        if (n == Base256(2) || n == Base256(3)) return true;
        if (n % Base256(2) == Base256(0)) return false;

        static const std::vector<uint32_t> smallPrimes = {
            3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
            73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
            157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
            239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317,
            331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419,
            421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499
        };
        for (const std::uint32_t p: smallPrimes) {
            Base256 primeObj(p);
            if (n == primeObj) return true;
            if (n % primeObj == Base256(0)) return false;
        }

        Base256 d = n - Base256(1);
        std::uint32_t s = 0;
        while (!isZero(d.getBytes()) && !isOdd(d)) {
            d = divideByTwo(d);
            s++;
        }

        BarrettReducer reducer(n);

        static const std::vector<std::uint32_t> bases = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};

        for (uint32_t baseVal: bases) {
            Base256 a(baseVal);
            if (a >= n) break;

            Base256 x = modPowWithReducer(a, d, reducer);

            if (x == Base256(1) || x == n - Base256(1)) {
                continue;
            }

            bool composite = true;
            for (uint32_t r = 1; r < s; r++) {
                x = reducer.reduce(x * x);

                if (x == n - Base256(1)) {
                    composite = false;
                    break;
                }
            }

            if (composite) {
                return false;
            }
        }

        return true;
    }

    Base256 pow(const Base256 &a, const std::uint64_t &power) {
        Base256 result(1);
        Base256 base = a;
        std::uint64_t p = power;
        while (p > 0) {
            if (p % 2 == 1) {
                result = result * base;
            }
            base = base * base;
            p /= 2;
        }
        return result;
    }

    Base256 modPow(Base256 base, Base256 exponent, const Base256 &modulus) {
        if (modulus <= Base256(1)) return Base256(0);

        BarrettReducer reducer(modulus);
        return modPowWithReducer(base, exponent, reducer);
    }
} // namespace operations::math
