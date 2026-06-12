# Base256

Base256 is a small C++20 library for unsigned big-integer arithmetic using
Base-256 byte storage. It was developed for use in
[RSA-Encryptor](https://github.com/ParallelEngineering/RSA-Encryptor) and is
intended to be integrated as a Git submodule.

## What it is for

The library provides arithmetic for numbers that can grow beyond native integer
limits such as `uint64_t`. This makes it useful for cryptographic-style
calculations where values often need more than the CPU's built-in integer width.

## How Base256 works

`operations::Base256` stores a number as a dynamically sized `ByteArray`
(`std::vector<uint8_t>`). Each byte is one Base-256 digit:

- index `0` contains the least-significant byte
- index `1` contains the next `256^1` byte
- higher indexes continue the same little-endian layout

After arithmetic operations, the internal byte array is normalized by removing
unused high zero-bytes while keeping zero represented as a single byte.

For a deeper explanation of the internal representation, see
[`docs/base256.md`](docs/base256.md).

## Usage

```cpp
#include "base256.h"

using operations::Base256;

int main() {
    Base256 a(4294967295ULL);
    Base256 b(2);

    Base256 sum = a + b;
    Base256 product = a * b;
    Base256 remainder = product % b;
    Base256 power = Base256::pow(b, 16);

    power.print();
}
```

The class supports construction from `uint64_t`, arithmetic operators
`+`, `-`, `*`, `/`, `%`, compound assignments, comparisons, `Base256::pow`, and
decimal output with `print()`.

## Integration

Add this repository to your parent project and include it with CMake:

```cmake
add_subdirectory(lib/Base256)
target_link_libraries(YourTarget PRIVATE Base256)
```

If your parent project does not already expose the library headers, add the
submodule's `src` directory to your include path.

> [!NOTE]
> Base256 is a library, not a standalone executable. The library code itself is
> meant to be used from another program. The tests, however, can be built and run
> directly from this repository without integrating Base256 into another project.

## Tests

Tests are enabled for Debug builds:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```


