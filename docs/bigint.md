# BigInt: Custom Big-Integer Mathematics

The **`BigInt`** class is a dynamically sized, arbitrary-precision integer implementation. It is designed to handle infinitely large unsigned integers by utilizing dynamically expanding contiguous memory (`std::vector<uint64_t>`).

By operating entirely in software, `BigInt` bypasses hardware ALU constraints (like standard 64-bit boundaries), making it suitable for cryptographic computations where numbers routinely span thousands of bits.

---

## Technical Architecture

### 1. The 64-Bit Limb System
Hardware natively computes in Base-2 (Binary), while human-readable formats use Base-10 (Decimal). This class uses a base-2^64 representation, treating exactly one `uint64_t` limb as a single, discrete digit.

Each digit holds a value from `0` to `2^64 - 1`. If an arithmetic operation pushes a limb beyond that range, it overflows and carries into the next limb. The total mathematical value is represented by the polynomial:

`Value = d[0]*((2^64)^0) + d[1]*((2^64)^1) + d[2]*((2^64)^2) ... + d[n]*((2^64)^n)`

### 2. Little-Endian Memory Mapping
Numbers are stored in **little-endian** order, meaning the least-significant limb is stored at index `0` of the vector.
Or in other words `(2^64)^0` is always stored at `data[0]`.

This architecture guarantees that the array index maps perfectly to the radix exponent. The limb at `data[i]` is strictly bound to `(2^64)^i`. This eliminates the need for complex index inversion during iterative algorithms.

| Number (Base 10) | Hexadecimal            | Limb Array `vector<uint64_t>` | Radix Evaluation                         |
|:-----------------|:-----------------------|:------------------------------|:-----------------------------------------|
| `42`             | `0x2A`                 | `[42]`                        | 42 * (2^64)^0                            |
| `2^64`           | `0x010000000000000000` | `[0, 1]`                      | 0 * (2^64)^0 + 1 * (2^64)^1              |
| `2^64 + 7`       | `0x010000000000000007` | `[7, 1]`                      | 7 * (2^64)^0 + 1 * (2^64)^1              |

### 3. Vector Normalization
Because operations like subtraction and division can shrink the magnitude of a number, mathematical leading zeros appear as trailing elements at the end of the little-endian vector.

The class enforces strict **Array Normalization**. After every operation, trailing zero-limbs are instantly popped from the vector until only the true magnitude remains (or until a single `[0]` is left). This ensures `data.size()` operates as a reliable magnitude heuristic and prevents logic failure during bitwise comparisons.

---

## Algorithmic Mechanics

The implementation provides addition, subtraction, multiplication, division, modulo, comparisons, decimal output, and selected helper functions in `operations::math`.

---

## Code Examples & Usage

The class acts like an unsigned primitive integer, natively supporting cross-limb cascading, memory-safe aliasing, and underflow clamping.

```cpp
#include "bigint.h"

using operations::BigInt;

// 1. Initialization
BigInt a(4294967295); // Initializes from max 32-bit int
BigInt b(2);

// 2. Infinite Precision Arithmetic
BigInt sum = a + b;   // Handles limb-overflows natively
BigInt prod = a * b;  // Expands underlying vector memory dynamically
BigInt quot = a / b;  // Evaluates using bitwise long-division

// 3. Safe Compound Assignment & Aliasing
a += b;
a *= a;                // Memory-safe aliasing (reads/writes safely isolated)

// 4. Edge-Case Safety
BigInt zero = b - a;  // Negative subtraction cleanly clamps to 0
BigInt drop = b / a;  // Fractional division drops remainder (evaluates to 0)

// 5. Comparisons & Output
if (sum > a && prod != zero) {
    sum.print();       // Iterates in reverse to print standard big-endian output
}
```
