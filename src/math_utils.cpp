#include "math_utils.h"

namespace operations::math {
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

    Base256 modInverse(Base256 a, Base256 m) {
        const Base256 m0 = m;
        Base256 y(0), x(1);

        if (m == Base256(1)) {
            return Base256{0};
        }

        bool signX = true;
        bool signY = true;

        while (a > Base256(1)) {
            Base256 q = a / m;
            Base256 t = m;

            m = a % m;
            a = t;
            t = y;

            Base256 qy = q * y;
            Base256 nextY;
            bool nextSignY;

            if (signX == signY) {
                if (x >= qy) {
                    nextY = x - qy;
                    nextSignY = signX;
                } else {
                    nextY = qy - x;
                    nextSignY = !signX;
                }
            } else {
                nextY = x + qy;
                nextSignY = signX;
            }

            x = t;
            signX = signY;

            y = nextY;
            signY = nextSignY;
        }

        if (!signX) {
            x = m0 - x;
        }

        return x;
    }

    bool isPrime(const Base256 &n) {
        if (n <= Base256(1)) return false;
        if (n == Base256(2) || n == Base256(3)) return true;
        if (n % Base256(2) == Base256(0)) return false;

        // Optimized division with small prime factors
        const std::uint64_t smallPrimes[] = {
            3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97
        };
        for (std::uint64_t p: smallPrimes) {
            Base256 primeVal(p);
            if (n == primeVal) return true;
            if (n % primeVal == Base256(0)) return false;
        }

        // Preparation for Miller-Rabin: n - 1 = d * 2^s
        Base256 d = n - Base256(1);
        size_t s = 0;
        while (d % Base256(2) == Base256(0)) {
            d /= Base256(2);
            s++;
        }

        // Testing with small prime bases
        const std::uint64_t bases[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
        for (const std::uint64_t base: bases) {
            Base256 a(base);
            if (a >= n) break;

            Base256 x = modPow(a, d, n);
            if (x == Base256(1) || x == n - Base256(1)) {
                continue;
            }

            bool composite = true;
            for (size_t r = 0; r < s - 1; ++r) {
                x = modPow(x, Base256(2), n);
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

    Base256 pow(const Base256 &a, const std::uint64_t &pow) {
        Base256 result(1);

        for (std::uint64_t i = 0; i < pow; ++i) {
            result *= a;
        }

        return result;
    }

    Base256 modPow(Base256 base, Base256 exponent, const Base256 &modulus) {
        Base256 result(1);
        base %= modulus;

        const Base256 zero(0);
        const Base256 one(1);
        const Base256 two(2);

        while (exponent > zero) {
            if (exponent % two == one) {
                result = (result * base) % modulus;
            }
            base = (base * base) % modulus;
            exponent /= two;
        }
        return result;
    }
} // namespace operations::math
