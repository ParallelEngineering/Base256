# Addition
The addition algorithm operates in Base $2^{64}$, using 64-bit unsigned integers (`uint64_t`) to represent each limb of the arbitrary-precision integer. Overflows beyond the 64-bit boundary are detected using standard unsigned arithmetic comparisons.

1. **Iteration:** The algorithm traverses from the least significant limb (LSB, `index 0`) to the most significant limb (MSB, `index N`).
2. **Carry Propagation:** For each limb, represented by `i`, it calculates:
   `sum = aVal + bVal`
   An overflow is detected if the intermediate `sum` is smaller than `aVal`.
3. **Carry Handling:**
   * The carry for the next iteration (`nextCarry`) is initially flagged as `1` if an overflow occurred in the initial sum.
   * The carry from the previous iteration is then added to the `sum`. If this addition causes a secondary overflow (detected by checking `sum < carry`), `nextCarry` is incremented.
4. **Finalization:** If a carry remains after the final limb is processed, an additional limb containing the value `1` is appended to the vector. Finally, the vector is normalized to prune trailing zeros.