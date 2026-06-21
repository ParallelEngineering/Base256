#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "base256.h"

namespace operations::math {

    // Computes the greatest common divisor of a and b
    Base256 gcd(const Base256 &a, const Base256 &b);

    // Computes the modular multiplicative inverse of a modulo m
    Base256 modInverse(Base256 a, Base256 m);

    // Performs the Miller-Rabin primality test on n
    bool isPrime(const Base256 &n);

    Base256 pow(const Base256 &a, const std::uint64_t &pow);

    Base256 modPow(Base256 base, Base256 exponent, const Base256 &modulus);

} // namespace operations::math

#endif // MATH_UTILS_H