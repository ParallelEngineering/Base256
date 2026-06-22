#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <iostream>
#include <limits>
#include <sstream>

#include "../src/base256.h"
#include "../src/math_utils.h"

using operations::Base256;
using namespace operations::math;

static Base256 make(const uint64_t v) { return Base256(v); }

// Helper for maximum uint64_t value
constexpr uint64_t MAX_U64 = std::numeric_limits<uint64_t>::max();

TEST_CASE("Base256: constructors, copy, assignment, self-assignment") {
    SECTION("Default is zero") {
        Base256 a;
        REQUIRE(a == make(0));
    }

    SECTION("Construct from uint64_t") {
        Base256 a(42);
        REQUIRE(a == make(42));
        REQUIRE(a != make(41));
    }

    SECTION("Construct from max uint64_t") {
        Base256 a(MAX_U64);
        REQUIRE(a == make(MAX_U64));
        REQUIRE(a > make(MAX_U64 - 1));
    }

    SECTION("Copy constructor copies value") {
        Base256 a(123456789);
        Base256 b(a);
        REQUIRE(b == a);
    }

    SECTION("Assignment copies value") {
        Base256 a(777);
        Base256 b(888);
        b = a;
        REQUIRE(b == a);
    }

    SECTION("Self-assignment is safe") {
        Base256 a(424242);
        Base256 &ref = a;
        a = ref;  // self-assign
        REQUIRE(a == make(424242));
    }
}

TEST_CASE("Base256: equality and inequality") {
    Base256 a(0), b(0), c(1), d(MAX_U64);
    REQUIRE(a == b);
    REQUIRE(a != c);
    REQUIRE(d == make(MAX_U64));
    REQUIRE(d != make(0));
}

TEST_CASE("Base256: addition & compound addition") {
    SECTION("Simple add") { REQUIRE(make(10) + make(20) == make(30)); }

    SECTION("Add zero is identity") {
        REQUIRE(make(123456) + make(0) == make(123456));
        REQUIRE(make(0) + make(123456) == make(123456));
    }

    SECTION("Carry across one byte") { REQUIRE(make(255) + make(1) == make(256)); }

    SECTION("Cascading carry across multiple bytes") {
        REQUIRE(make(65535) + make(1) == make(65536));            // 0xFFFF + 1 = 0x10000
        REQUIRE(make(16777215) + make(1) == make(16777216));      // 0xFFFFFF + 1
        REQUIRE(make(4294967295) + make(1) == make(4294967296));  // 0xFFFFFFFF + 1
    }

    SECTION("Addition exceeding uint64_t (8 bytes to 9 bytes)") {
        Base256 a(MAX_U64);  // 0xFFFFFFFFFFFFFFFF
        Base256 b(2);
        Base256 c = a + b;

        // We can't verify with make() because it exceeds uint64_t.
        // But we know (MAX_U64 + 2) - 2 should be MAX_U64
        REQUIRE(c - b == a);
        REQUIRE(c > a);
    }

    SECTION("Compound += operator") {
        Base256 a(100);
        a += make(50);
        REQUIRE(a == make(150));
        a += make(0);
        REQUIRE(a == make(150));
    }

    SECTION("Commutativity: a + b == b + a (small domain)") {
        for (uint64_t a = 0; a <= 200; ++a) {
            for (uint64_t b = 0; b <= 200; ++b) {
                REQUIRE(make(a) + make(b) == make(b) + make(a));
            }
        }
    }
}

TEST_CASE("Base256: subtraction & compound subtraction") {
    SECTION("Simple sub") { REQUIRE(make(30) - make(20) == make(10)); }

    SECTION("Subtract zero is identity") { REQUIRE(make(123456) - make(0) == make(123456)); }

    SECTION("Subtract to zero") {
        REQUIRE(make(123456) - make(123456) == make(0));
        REQUIRE(make(MAX_U64) - make(MAX_U64) == make(0));
    }

    SECTION("Borrow across one byte") { REQUIRE(make(256) - make(1) == make(255)); }

    SECTION("Cascading borrow across multiple bytes") {
        REQUIRE(make(65536) - make(1) == make(65535));
        REQUIRE(make(16777216) - make(1) == make(16777215));
        REQUIRE(make(4294967296) - make(1) == make(4294967295));
    }

    SECTION("Negative result clamping (Underflow to 0)") {
        // Based on your comment: "if it would be negative, 0 is returned"
        REQUIRE(make(10) - make(20) == make(0));
        REQUIRE(make(0) - make(1) == make(0));
        REQUIRE(make(500) - make(MAX_U64) == make(0));
    }

    SECTION("Compound -= operator") {
        Base256 a(100);
        a -= make(40);
        REQUIRE(a == make(60));
        a -= make(60);
        REQUIRE(a == make(0));
    }
}

TEST_CASE("Base256: multiplication & compound multiplication") {
    SECTION("Simple mul") { REQUIRE(make(7) * make(6) == make(42)); }

    SECTION("Mul by zero and one") {
        REQUIRE(make(123456) * make(0) == make(0));
        REQUIRE(make(0) * make(123456) == make(0));
        REQUIRE(make(123456) * make(1) == make(123456));
        REQUIRE(make(1) * make(123456) == make(123456));
    }

    SECTION("Cross-byte product") {
        REQUIRE(make(256) * make(256) == make(65536));
        REQUIRE(make(65536) * make(256) == make(16777216));
    }

    SECTION("Large multiplication (Exceeding uint64_t)") {
        Base256 a(4294967295);  // 0xFFFFFFFF (4 bytes)
        Base256 b(4294967295);
        Base256 c = a * b;  // Should be 0xFFFFFFFE00000001 (8 bytes)

        // Verify via division
        REQUIRE(c / a == b);
        REQUIRE(c / b == a);
    }

    SECTION("Compound *= operator") {
        Base256 a(10);
        a *= make(10);
        REQUIRE(a == make(100));
        a *= make(0);
        REQUIRE(a == make(0));
    }

    SECTION("Distributivity: a*(b+c) == a*b + a*c (very small domain)") {
        for (uint64_t a = 0; a <= 10; ++a)
            for (uint64_t b = 0; b <= 10; ++b)
                for (uint64_t c = 0; c <= 10; ++c)
                    REQUIRE(make(a) * (make(b) + make(c)) ==
                            (make(a) * make(b)) + (make(a) * make(c)));
    }
}

TEST_CASE("Base256: division & compound division") {
    SECTION("Exact division") {
        REQUIRE(make(42) / make(6) == make(7));
        REQUIRE(make(65536) / make(256) == make(256));
    }

    SECTION("Identity and Zero Numerator") {
        REQUIRE(make(123456) / make(1) == make(123456));
        REQUIRE(make(MAX_U64) / make(1) == make(MAX_U64));
        REQUIRE(make(0) / make(123456) == make(0));
    }

    SECTION("Fractional Division (Remainder dropped)") {
        REQUIRE(make(10) / make(3) == make(3));
        REQUIRE(make(255) / make(2) == make(127));
        REQUIRE(make(1000) / make(333) == make(3));
    }

    SECTION("Dividend smaller than Divisor (Results in 0)") {
        REQUIRE(make(10) / make(20) == make(0));
        REQUIRE(make(255) / make(256) == make(0));
        REQUIRE(make(1) / make(MAX_U64) == make(0));
    }

    SECTION("Compound /= operator") {
        Base256 a(100);
        a /= make(2);
        REQUIRE(a == make(50));
        a /= make(50);
        REQUIRE(a == make(1));
    }
}

TEST_CASE("Base256: division by zero (edge cases)") {
    // Tests the left side of your OR statement: `isZero(divisor)`
    SECTION("Dividing a normal number by zero yields zero") {
        REQUIRE(make(42) / make(0) == make(0));
        REQUIRE(make(MAX_U64) / make(0) == make(0));
    }

    SECTION("Dividing zero by zero yields zero") { REQUIRE(make(0) / make(0) == make(0)); }
}

TEST_CASE("Base256: remainder / modulo logic") {
    // Assuming you have implemented operator% (e.g., a % b)
    // If not, adapt this to test your exposed remainder API or `div()` method

    SECTION("Remainder of zero dividend yields zero") {
        // Tests the interior block: `if (remaining != nullptr) *remaining = {0};`
        // when `initialDividendIndex < 0`
        REQUIRE(make(0) % make(123456) == make(0));
    }

    SECTION("Remainder of division by zero yields zero") {
        // Tests the interior block: `if (remaining != nullptr) *remaining = {0};`
        // when `isZero(divisor)`
        REQUIRE(make(123456) % make(0) == make(0));
    }

    SECTION("Normal fractional remainders") {
        // Validates standard operation of the `remaining` pointer during `div`
        REQUIRE(make(10) % make(3) == make(1));
        REQUIRE(make(255) % make(2) == make(1));
        REQUIRE(make(1000) % make(333) == make(1));
    }

    SECTION("Dividend smaller than divisor yields the dividend as remainder") {
        REQUIRE(make(10) % make(20) == make(10));
        REQUIRE(make(1) % make(MAX_U64) == make(1));
    }
}

TEST_CASE("Base256: mixed expressions & combinations") {
    SECTION("(a + b) * c then divide back by c (c > 0)") {
        for (uint64_t a = 1; a <= 20; ++a)
            for (uint64_t b = 1; b <= 20; ++b)
                for (uint64_t c = 1; c <= 20; ++c) {
                    Base256 additionResult = make(a) + make(b);
                    Base256 expr = additionResult * make(c);
                    Base256 divisionResult = expr / make(c);
                    REQUIRE(divisionResult == additionResult);
                }
    }

    SECTION("Chained arithmetic: ((100 * 256) + 50 - 25) / 5 == 5125") {
        Base256 result = ((make(100) * make(256)) + make(50) - make(25)) / make(5);
        REQUIRE(result == make(5125));
    }
}

TEST_CASE("Base256: comparisons edge cases") {
    Base256 a0(0), a1(1), a2(2), a255(255), a256(256);
    Base256 max64(MAX_U64);

    SECTION("Basic ordering") {
        REQUIRE(a0 < a1);
        REQUIRE(a1 < a2);
        REQUIRE(a255 < a256);

        REQUIRE(a256 > a255);
        REQUIRE(a2 > a1);
        REQUIRE(a1 > a0);
    }

    SECTION("Equality vs ordering on same values") {
        Base256 x(123456);
        Base256 y(123456);
        REQUIRE(x == y);
        REQUIRE_FALSE(x != y);

        // Strictness
        REQUIRE_FALSE(x < x);
        REQUIRE(x <= x);
        REQUIRE_FALSE(x > x);
        REQUIRE(x >= x);

        REQUIRE_FALSE(x < y);
        REQUIRE(x <= y);
        REQUIRE_FALSE(x > y);
        REQUIRE(x >= y);
    }

    SECTION("Cross-byte comparisons") {
        REQUIRE(make(256) > make(255));
        REQUIRE(make(65536) > make(65535));
        REQUIRE(make(65536) >= make(65536));
        REQUIRE_FALSE(make(65536) < make(65536));
        REQUIRE(make(16777216) > make(16777215));
    }

    SECTION("Extremely large comparisons") {
        REQUIRE(max64 > a256);
        REQUIRE(a0 < max64);

        Base256 overflow = max64 + a1;  // Exceeds uint64_t
        REQUIRE(overflow > max64);
        REQUIRE(max64 < overflow);
    }
}

TEST_CASE("Base256: print outputs base10 string to cout") {
    // Helper lambda to capture std::cout output safely
    auto capturePrint = [](const Base256 &num) {
        std::stringstream buffer;
        // Redirect std::cout to our buffer
        std::streambuf *oldCout = std::cout.rdbuf(buffer.rdbuf());
        num.print();
        // Restore std::cout to its original state
        std::cout.rdbuf(oldCout);
        return buffer.str();
    };

    SECTION("Absolute zero") { REQUIRE(capturePrint(make(0)) == "0\n"); }

    SECTION("Single digit / small numbers") {
        REQUIRE(capturePrint(make(7)) == "7\n");
        REQUIRE(capturePrint(make(42)) == "42\n");
    }

    SECTION("Byte boundary overlaps") {
        REQUIRE(capturePrint(make(255)) == "255\n");
        REQUIRE(capturePrint(make(256)) == "256\n");
        REQUIRE(capturePrint(make(65535)) == "65535\n");
        REQUIRE(capturePrint(make(65536)) == "65536\n");
    }

    SECTION("Standard large uint64_t numbers") {
        REQUIRE(capturePrint(make(1000000000)) == "1000000000\n");
        // MAX_U64
        REQUIRE(capturePrint(make(MAX_U64)) == "18446744073709551615\n");
    }

    SECTION("Extreme cases: Exceeding uint64_t limitations") {
        // MAX_U64 + 1
        Base256 overflowPlusOne = make(MAX_U64) + make(1);
        REQUIRE(capturePrint(overflowPlusOne) == "18446744073709551616\n");

        // MAX_U64 * 10
        Base256 overflowTimesTen = make(MAX_U64) * make(10);
        REQUIRE(capturePrint(overflowTimesTen) == "184467440737095516150\n");
    }
}

TEST_CASE("Base256: power edge cases") {
    // Test 255^2 (Should be 65025, or [0x01, 0xFE] in little-endian)
    Base256 res = pow(Base256(255), 2);
    REQUIRE(res == Base256(65025));

    // Test a power that results in a huge number of trailing zeros in Base-256
    // 256^2 should be [0, 0, 1]
    Base256 largePower = pow(Base256(2), 16);
    REQUIRE(largePower == Base256(65536));
}

TEST_CASE("Base256: modPow (modular exponentiation)") {
    SECTION("Known small results") {
        REQUIRE(modPow(make(2), make(10), make(1000)) == make(24));
        REQUIRE(modPow(make(7), make(3), make(13)) == make(5));
        REQUIRE(modPow(make(12), make(7), make(13)) == make(12));
    }

    SECTION("Exponent zero yields multiplicative identity") {
        REQUIRE(modPow(make(5), make(0), make(13)) == make(1));
        REQUIRE(modPow(make(0), make(0), make(13)) == make(1));
    }

    SECTION("Zero base stays zero for positive exponents") {
        REQUIRE(modPow(make(0), make(5), make(13)) == make(0));
        REQUIRE(modPow(make(0), make(64), make(17)) == make(0));
    }

    SECTION("Larger exponent still reduces correctly") {
        REQUIRE(modPow(make(2), make(64), make(17)) == make(1));
    }
}

TEST_CASE("Base256: pow (exponentiation)") {
    SECTION("Power of zero (x^0 = 1)") {
        REQUIRE(pow(make(10), 0) == make(1));
        REQUIRE(pow(make(255), 0) == make(1));
        REQUIRE(pow(make(MAX_U64), 0) == make(1));
        // Mathematical convention for programming: 0^0 = 1
        REQUIRE(pow(make(0), 0) == make(1));
    }

    SECTION("Power of one (x^1 = x)") {
        REQUIRE(pow(make(0), 1) == make(0));
        REQUIRE(pow(make(42), 1) == make(42));
        REQUIRE(pow(make(MAX_U64), 1) == make(MAX_U64));
    }

    SECTION("Zero to the power of X (0^x = 0)") {
        REQUIRE(pow(make(0), 2) == make(0));
        REQUIRE(pow(make(0), 100) == make(0));
    }

    SECTION("One to the power of X (1^x = 1)") {
        REQUIRE(pow(make(1), 2) == make(1));
        REQUIRE(pow(make(1), 100) == make(1));
    }

    SECTION("Standard combinations") {
        REQUIRE(pow(make(2), 2) == make(4));
        REQUIRE(pow(make(2), 8) == make(256));
        REQUIRE(pow(make(2), 16) == make(65536));
        REQUIRE(pow(make(10), 3) == make(1000));
        REQUIRE(pow(make(5), 4) == make(625));
    }

    SECTION("Extreme case: 2^64 perfectly overflows 64-bit boundaries") {
        // 2^64 evaluates to 18,446,744,073,709,551,616
        // Which is exactly MAX_U64 + 1
        Base256 twoTo64 = pow(make(2), 64);
        REQUIRE(twoTo64 == make(MAX_U64) + make(1));
    }

    SECTION("Extreme case: 10^20 using print verification") {
        // 10^19 fits in uint64_t. 10^20 does not.
        Base256 tenTo20 = pow(make(10), 20);

        std::stringstream buffer;
        std::streambuf *oldCout = std::cout.rdbuf(buffer.rdbuf());
        tenTo20.print();
        std::cout.rdbuf(oldCout);

        REQUIRE(buffer.str() == "100000000000000000000\n");
    }

    SECTION("Extreme case: Large base, small exponent (256^4)") {
        // 256^4 results in a very specific memory layout in Little-Endian Base-256
        // It requires exactly 5 bytes: [0, 0, 0, 0, 1]
        Base256 res = pow(make(256), 4);

        // We can verify this computationally by dividing by 256 four times
        REQUIRE(res / make(256) / make(256) / make(256) / make(256) == make(1));
    }
}

TEST_CASE("Base256 Math Utils: gcd (Greatest Common Divisor)") {
    SECTION("GCD of zero and a number is the number itself") {
        REQUIRE(gcd(make(42), make(0)) == make(42));
        REQUIRE(gcd(make(0), make(42)) == make(42));
        REQUIRE(gcd(make(0), make(0)) == make(0));
    }

    SECTION("GCD of prime numbers is 1 (coprime)") {
        // 101 and 103 are both prime numbers
        REQUIRE(gcd(make(101), make(103)) == make(1));
    }

    SECTION("GCD with a common factor") {
        // Both 12 and 15 share the factor 3
        REQUIRE(gcd(make(12), make(15)) == make(3));
    }

    SECTION("GCD of larger numbers") {
        // Verified: gcd(1234567890, 987654321) = 9
        REQUIRE(gcd(make(1234567890), make(987654321)) == make(9));
    }
}

TEST_CASE("Base256 Math Utils: modInverse (Modular Multiplicative Inverse)") {
    SECTION("Small modular inverse test cases") {
        // Verified: (3 * 4) % 11 = 12 % 11 = 1
        REQUIRE(modInverse(make(3), make(11)) == make(4));

        // Verified: (7 * 23) % 40 = 161 % 40 = 1
        REQUIRE(modInverse(make(7), make(40)) == make(23));
    }

    SECTION("Edge case: Modulus is 1 always yields 0") {
        REQUIRE(modInverse(make(5), make(1)) == make(0));
    }

    SECTION("Large RSA-like modular inverse case") {
        // Verified with primes p = 1000003 and q = 1000033
        // phi = (p - 1) * (q - 1) = 1000034000064
        // e = 65537
        // d = modInverse(e, phi) = 983264276609
        Base256 phi(1000034000064ULL);
        Base256 e(65537);
        Base256 d = modInverse(e, phi);

        REQUIRE(d == make(983264276609ULL));

        // Double check: (e * d) % phi should equal 1
        REQUIRE((e * d) % phi == make(1));
    }
}

TEST_CASE("Base256 Math Utils: isPrime (Primality Testing)") {
    SECTION("Numbers less than or equal to 1 are not prime") {
        REQUIRE_FALSE(isPrime(make(0)));
        REQUIRE_FALSE(isPrime(make(1)));
    }

    SECTION("Small known prime numbers") {
        REQUIRE(isPrime(make(2)));
        REQUIRE(isPrime(make(3)));
        REQUIRE(isPrime(make(5)));
        REQUIRE(isPrime(make(97)));
        REQUIRE(isPrime(make(1000003)));
    }

    SECTION("Small known composite numbers") {
        REQUIRE_FALSE(isPrime(make(4)));
        REQUIRE_FALSE(isPrime(make(9)));
        REQUIRE_FALSE(isPrime(make(15)));
        REQUIRE_FALSE(isPrime(make(100)));
        REQUIRE_FALSE(isPrime(make(1000005)));
    }

    SECTION("Large prime and composite numbers near 32-bit boundary") {
        // 4294967291 is the largest prime number below 2^32
        REQUIRE(isPrime(make(4294967291ULL)));

        // 4294967293 is composite (9241 * 464773)
        // It does not have small prime factors, forcing a thorough Miller-Rabin test
        REQUIRE_FALSE(isPrime(make(4294967293ULL)));
    }
}

TEST_CASE("Base256: Performance Benchmarks", "[.][benchmark]") {
    // 2048-Bit-Numbers (256 Bytes)
    std::vector<uint8_t> bytesA(256, 0xAA);
    std::vector<uint8_t> bytesB(256, 0x55);

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