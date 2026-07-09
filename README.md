# BigInt

BigInt is a small C++20 library for unsigned big-integer arithmetic using
64-bit limb storage. It was developed for use in
[RSA-Encryptor](https://github.com/ParallelEngineering/RSA-Encryptor) and is
intended to be integrated as a Git submodule.

## What it is for

The library provides arithmetic for numbers that can grow beyond native integer
limits such as `uint64_t`. This makes it useful for cryptographic-style
calculations where values often need more than the CPU's built-in integer width.

## How BigInt works

`operations::BigInt` stores a number as a dynamically sized `ByteArray`
(`std::vector<uint64_t>`). Each limb is one base-2^64 digit:

- index `0` contains the least-significant limb
- index `1` contains the next `(2^64)^1` limb
- higher indexes continue the same little-endian layout

After arithmetic operations, the internal limb array is normalized by removing
unused high zero-limbs while keeping zero represented as a single limb.

For a deeper explanation of the internal representation, see
[`docs/bigint.md`](docs/bigint.md).

## Usage

```cpp
#include "bigint.h"
#include "math_utils.h"

using operations::BigInt;
using operations::math::pow;

int main() {
    BigInt a(4294967295ULL);
    BigInt b(2);

    BigInt sum = a + b;
    BigInt product = a * b;
    BigInt remainder = product % b;
    BigInt power = pow(b, 16);

    power.print();
}
```

The class supports construction from `uint64_t`, arithmetic operators
`+`, `-`, `*`, `/`, `%`, compound assignments, comparisons, `operations::math::pow`, and
decimal output with `print()`.

## Integration

Add this repository to your parent project and include it with CMake:

```cmake
add_subdirectory(lib/BigInt)
target_link_libraries(YourTarget PRIVATE BigInt)
```

If your parent project does not already expose the library headers, add the
submodule's `src` directory to your include path.

> [!NOTE]
> BigInt is a library, not a standalone executable. The library code itself is
> meant to be used from another program. The tests, however, can be built and run
> directly from this repository without integrating BigInt into another project.

## Tests

Tests are enabled for Debug builds:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```
