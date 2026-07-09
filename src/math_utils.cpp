#include "math_utils.h"
#include <vector>
#include "helper.h"

namespace operations::math {

    namespace {
        // Shift a BigInt right by n limbs, dividing by (2^64)^n.
        BigInt shiftRightLimbs(const BigInt &val, size_t n) {
            const auto &limbs = val.getBytes();
            if (n >= limbs.size()) {
                return BigInt(0);
            }
            ByteArray newLimbs(limbs.begin() + n, limbs.end());
            return BigInt(newLimbs);
        }

        // Barrett Reducer encapsulation to replace slow divisions
        struct BarrettReducer {
            BigInt modulus;
            size_t k;
            BigInt mu;

            BarrettReducer(const BigInt &mod) : modulus(mod) {
                k = modulus.getBytes().size();

                // Create (2^64)^(2k) as [0, 0, ..., 1] in little-endian representation.
                ByteArray tempLimbs(2 * k + 1, 0);
                tempLimbs.back() = 1;
                BigInt bigPower(tempLimbs);

                // Compute mu = floor((2^64)^(2k) / modulus).
                // This is the only division performed during the entire modPow operation
                mu = bigPower / modulus;
            }

            // Reduces x modulo modulus, assuming x < modulus^2
            BigInt reduce(const BigInt &x) const {
                if (x < modulus) {
                    return x;
                }

                // q1 = x >> (k-1)
                size_t shift1 = (k > 1) ? (k - 1) : 0;
                BigInt q1 = shiftRightLimbs(x, shift1);

                // q2 = q1 * mu
                BigInt q2 = q1 * mu;

                // q3 = q2 >> (k+1)
                BigInt q3 = shiftRightLimbs(q2, k + 1);

                // r = x - q3 * modulus
                BigInt r = x - (q3 * modulus);

                // Since q3 is an approximation, we adjust at most twice
                while (r >= modulus) {
                    r -= modulus;
                }
                return r;
            }
        };
    }

    bool isOdd(const BigInt &val) {
        const auto &bytes = val.getBytes();
        if (bytes.empty()) return false;
        return (bytes[0] & 1) != 0;
    }

    BigInt divideByTwo(const BigInt &val) {
        auto bytes = val.getBytes();
        if (bytes.empty()) return BigInt(0);

        std::uint64_t carry = 0;

        for (int i = static_cast<int>(bytes.size()) - 1; i >= 0; --i) {
            const std::uint64_t next_carry = (bytes[i] & 1) ? 0x8000'0000'0000'0000ULL : 0;
            bytes[i] = (bytes[i] >> 1) | carry;
            carry = next_carry;
        }

        BigInt::normalizeVector(bytes);

        return BigInt(bytes);
    }

    BigInt gcd(const BigInt &a, const BigInt &b) {
        BigInt tempA = a;
        BigInt tempB = b;
        while (tempB != BigInt(0)) {
            const BigInt t = tempB;
            tempB = tempA % tempB;
            tempA = t;
        }
        return tempA;
    }

    BigInt modInverse(const BigInt &a, const BigInt &m) {
        BigInt r0 = m;
        BigInt r1 = a;
        BigInt x0(0);
        BigInt x1(1);

        bool sign0 = true;
        bool sign1 = true;

        while (r1 != BigInt(0)) {
            BigInt q = r0 / r1;
            BigInt r2 = r0 % r1;

            BigInt x2(0);
            bool sign2 = true;

            if (sign0 == sign1) {
                BigInt qx = q * x1;
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

        if (r0 != BigInt(1)) {
            return BigInt(0);
        }

        if (!sign0) {
            return m - (x0 % m);
        }
        return x0 % m;
    }

    bool isPrime(const BigInt &n) {
        if (n <= BigInt(1)) return false;
        if (n == BigInt(2) || n == BigInt(3)) return true;
        if (n % BigInt(2) == BigInt(0)) return false;

        static const std::vector<uint32_t> smallPrimes = {
            3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
            73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
            157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
            239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317,
            331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419,
            421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499
        };
        for (const std::uint32_t p: smallPrimes) {
            BigInt primeObj(p);
            if (n == primeObj) return true;
            if (n % primeObj == BigInt(0)) return false;
        }

        BigInt d = n - BigInt(1);
        std::uint32_t s = 0;
        while (!isZero(d.getBytes()) && !isOdd(d)) {
            d = divideByTwo(d);
            s++;
        }

        static const std::vector<std::uint32_t> bases = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};

        for (uint32_t baseVal: bases) {
            BigInt a(baseVal);
            if (a >= n) break;

            BigInt x = modPow(a, d, n);

            if (x == BigInt(1) || x == n - BigInt(1)) {
                continue;
            }

            bool composite = true;
            for (uint32_t r = 1; r < s; r++) {
                x = modPow(x, BigInt(2), n);
                if (x == n - BigInt(1)) {
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

    BigInt pow(const BigInt &a, const std::uint64_t &power) {
        BigInt result(1);
        BigInt base = a;
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

    BigInt modPow(BigInt base, BigInt exponent, const BigInt &modulus) {
        if (modulus <= BigInt(1)) return BigInt(0);

        BigInt result(1);

        // Initialize Barrett Reducer.
        // This does exactly one slow division to precompute mu.
        BarrettReducer reducer(modulus);

        // Use fast reduction instead of % operator
        base = reducer.reduce(base);

        while (!isZero(exponent.getBytes())) {
            if (isOdd(exponent)) {
                // Reduces result * base without division
                result = reducer.reduce(result * base);
            }
            // Reduces base * base without division
            base = reducer.reduce(base * base);
            exponent = divideByTwo(exponent);
        }
        return result;
    }
} // namespace operations::math
