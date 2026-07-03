#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <iostream>

#include "../src/base256.h"
#include "../src/math_utils.h"

using operations::Base256;
using namespace operations::math;

TEST_CASE("Base256: Performance Benchmarks", "[benchmark]") {
    // 2048-Bit-Numbers (256 Bytes)
    ByteArray bytesA(256, 0xAA);
    ByteArray bytesB(256, 0x55);

    Base256 largeNum(bytesA);
    Base256 divisor(3);
    Base256 largeDivisor(bytesB);

    BENCHMARK("Division: 2048-Bit / 3") {
        return largeNum / divisor;
    };

    BENCHMARK("Division: 2048-Bit / 2048-Bit") {
        return largeNum / largeDivisor;
    };

    BENCHMARK("ModPow: 2048-Bit ^ 65537 mod 2048-Bit (RSA)") {
        Base256 exponent(65537);
        return modPow(largeNum, exponent, largeDivisor);
    };
}

TEST_CASE("Base256: modPow Performance Benchmarks", "[.][benchmark][modpow]") {
    ByteArray base_bytes(32, 0xAA);
    ByteArray exp_bytes(32, 0x55);
    ByteArray mod_bytes(32, 0xFF);

    Base256 base(base_bytes);
    Base256 exponent_large(exp_bytes);
    Base256 exponent_small(65537);
    Base256 modulus(mod_bytes);

    BENCHMARK("modPow: 2048-Bit ^ 65537 mod 2048-Bit (Encryption)") {
        return modPow(base, exponent_small, modulus);
    };

    BENCHMARK("modPow: 2048-Bit ^ 2048-Bit mod 2048-Bit (Decryption)") {
        return modPow(base, exponent_large, modulus);
    };
}

TEST_CASE("Base256: Core Arithmetic Benchmarks", "[benchmark][arithmetic]") {
    ByteArray wordsA(32, 0xAAAAAAAAAAAAAAAAULL);
    ByteArray wordsB(32, 0x5555555555555555ULL);

    Base256 numA(wordsA);
    Base256 numB(wordsB);

    BENCHMARK("Addition: 2048-Bit + 2048-Bit") {
        return numA + numB;
    };

    BENCHMARK("Subtraction: 2048-Bit - 2048-Bit") {
        return numA - numB;
    };

    BENCHMARK("Multiplication: 2048-Bit * 2048-Bit") {
        return numA * numB;
    };

    BENCHMARK("Division: 2048-Bit / 2048-Bit") {
        return numA / numB;
    };
}

TEST_CASE("Base256: Key Generation Math Benchmarks", "[benchmark][keygen]") {
    ByteArray wordsPhi(32, 0xBBBBBBBBBBBBBBBBULL);
    Base256 phi(wordsPhi);
    Base256 e(65537);

    BENCHMARK("GCD: gcd(2048-Bit, 65537)") {
        return gcd(phi, e);
    };

    BENCHMARK("modInverse: modInverse(65537, 2048-Bit)") {
        return modInverse(e, phi);
    };

    ByteArray wordsCandidate(32, 0xDDDDDDDDDDDDDDDDULL);
    wordsCandidate[0] |= 0x01;
    Base256 candidate(wordsCandidate);

    BENCHMARK("isPrime: 2048-Bit") {
        return isPrime(candidate);
    };
}

TEST_CASE("Base256: Size Comparison (2048-Bit vs. 16384-Bit)", "[benchmark][comparison]") {
    ByteArray words2048(32, 0xAAAAAAAAAAAAAAAAULL);
    Base256 base2048(words2048);
    Base256 mod2048(words2048);
    mod2048 += Base256(1);

    ByteArray words16384(256, 0xAAAAAAAAAAAAAAAAULL);
    Base256 base16384(words16384);
    Base256 mod16384(words16384);
    mod16384 += Base256(1);

    Base256 exponent(65537);

    BENCHMARK("modPow: Real 2048-Bit (RSA-Encryption)") {
        return modPow(base2048, exponent, mod2048);
    };

    BENCHMARK("modPow: Wrong 16384-Bit (Bug case)") {
        return modPow(base16384, exponent, mod16384);
    };
}