#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "bigint.h"

namespace operations::math {

    bool isOdd(const BigInt &val);
    BigInt divideByTwo(const BigInt &val);
    // Computes the greatest common divisor of a and b
    BigInt gcd(const BigInt &a, const BigInt &b);

    // Computes the modular multiplicative inverse of a modulo m
    BigInt modInverse(const BigInt &a, const BigInt &m);

    // Performs the Miller-Rabin primality test on n
    bool isPrime(const BigInt &n);

    BigInt pow(const BigInt &a, const std::uint64_t &pow);

    BigInt modPow(BigInt base, BigInt exponent, const BigInt &modulus);

} // namespace operations::math

#endif // MATH_UTILS_H