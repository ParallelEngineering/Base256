# Base256: Custom Big-Integer Mathematics

The **`Base256`** class is a dynamically sized, arbitrary-precision integer implementation. It is designed to handle infinitely large unsigned integers by utilizing dynamically expanding contiguous memory (`std::vector<uint8_t>`).

By operating entirely in software, `Base256` bypasses hardware ALU constraints (like standard 64-bit boundaries), making it suitable for cryptographic computations where numbers routinely span thousands of bits.

---

## Technical Architecture

### 1. The Radix-256 (Base-256) System
Hardware natively computes in Base-2 (Binary), while human-readable formats use Base-10 (Decimal). This class uses **Base-256**, treating exactly **one byte** (`uint8_t`) as a single, discrete "digit."

Each digit holds a value from `0` to `255`. If an arithmetic operation pushes a byte beyond 255, it overflows and carries a `1` into the next byte magnitude. The total mathematical value is represented by the polynomial:

`Value = d[0]*(256^0) + d[1]*(256^1) + d[2]*(256^2) ... + d[n]*(256^n)`

### 2. Little-Endian Memory Mapping
Numbers are stored in **little-endian** order, meaning the least-significant byte (LSB) is stored at index `0` of the vector.
Or in other words 256^0 is always stored at `data[0]`.

This architecture guarantees that the array index maps perfectly to the radix exponent. The byte at `data[i]` is strictly bound to `256^i`. This eliminates the need for complex index inversion during iterative algorithms.

| Number (Base 10) | Hexadecimal | Base-256 Array `vector<uint8_t>` | Radix Evaluation                  |
|:-----------------|:------------|:---------------------------------|:----------------------------------|
| `42`             | `0x2A`      | `[42]`                           | 42 * 256^0                        |
| `258`            | `0x0102`    | `[2, 1]`                         | 2 * 256^0 + 1 * 256^1             |
| `65536`          | `0x010000`  | `[0, 0, 1]`                      | 0 * 256^0 + 0 * 256^1 + 1 * 256^2 |

### 3. Vector Normalization
Because operations like subtraction and division can shrink the magnitude of a number, mathematical leading zeros appear as trailing elements at the end of the little-endian vector.

The class enforces strict **Array Normalization**. After every operation, trailing zero-bytes are instantly popped from the vector until only the true magnitude remains (or until a single `[0]` is left). This ensures `data.size()` operates as a reliable magnitude heuristic and prevents logic failure during bitwise comparisons.

---

## Algorithmic Mechanics

- [Addition](algorithms/add.md#addition)
- [Subtraction](algorithms/sub.md#subtraction)
- [Subtraction](algorithms/mul.md#multiplication)
- [Subtraction](algorithms/div.md#division)
---

## Code Examples & Usage

The class acts identically to standard primitive integers, natively supporting cross-byte cascading, memory-safe aliasing, and underflow clamping.

```cpp
#include "vec/operations.h"
using operations::Base256;

// 1. Initialization
Base256 a(4294967295); // Initializes from max 32-bit int
Base256 b(2);

// 2. Infinite Precision Arithmetic
Base256 sum = a + b;   // Handles byte-overflows natively
Base256 prod = a * b;  // Expands underlying vector memory dynamically
Base256 quot = a / b;  // Evaluates using bitwise long-division

// 3. Safe Compound Assignment & Aliasing
a += b;
a *= a;                // Memory-safe aliasing (reads/writes safely isolated)

// 4. Edge-Case Safety
Base256 zero = b - a;  // Negative subtraction cleanly clamps to 0
Base256 drop = b / a;  // Fractional division drops remainder (evaluates to 0)

// 5. Comparisons & Output
if (sum > a && prod != zero) {
    sum.print();       // Iterates in reverse to print standard Big-Endian output
}
```