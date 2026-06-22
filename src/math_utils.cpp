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
        // Basisprüfungen
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
        for (uint32_t p : smallPrimes) {
            Base256 primeObj(p);
            if (n == primeObj) return true;
            if (n % primeObj == Base256(0)) return false;
        }

        // 2. Zerlegung: Schreibe n - 1 als d * 2^s
        Base256 d = n - Base256(1);
        uint32_t s = 0;
        while (d % Base256(2) == Base256(0)) {
            d = d / Base256(2);
            s++;
        }

        static const std::vector<uint32_t> bases = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};

        for (uint32_t baseVal : bases) {
            Base256 a(baseVal);
            if (a >= n) break;

            Base256 x = modPow(a, d, n);

            if (x == Base256(1) || x == n - Base256(1)) {
                continue;
            }

            bool composite = true;
            for (uint32_t r = 1; r < s; r++) {
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
        if (modulus == Base256(1)) return Base256(0);

        Base256 result(1);
        base = base % modulus;

        while (exponent > Base256(0)) {
            if (exponent % Base256(2) == Base256(1)) {
                result = (result * base) % modulus;
            }
            base = (base * base) % modulus;
            exponent = exponent / Base256(2);
        }
        return result;
    }

} // namespace operations::math
