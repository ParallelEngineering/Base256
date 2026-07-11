# Subtraction
Subtraction implements a "borrow" propagation system to ensure mathematical correctness within an unsigned 64-bit integer space:

1. **Underflow Protection:** Before processing, `isBigger` is checked. If the subtrahend `b` is larger than the minuend `a`, the operation clamps and returns `0` (or assigns `{0}` in-place) because negative integers are not supported.
2. **Borrow Propagation:** During limb-by-limb iteration, the algorithm calculates `diff = aVal - bVal`. An underflow is flagged (`nextBorrow = true`) if `aVal < bVal`.
3. **Regrouping:** If an underflow occurs, the algorithm effectively borrows $2^{64}$ (the base value) from the next higher limb (`index i + 1`). When subtracting the previous borrow from the current difference, a secondary underflow check (`diff < borrowVal`) ensures the borrow state is correctly propagated to the next iteration.
4. **Normalization:** Post-calculation, `normalizeVector` is called to prune trailing zero-limbs, keeping the internal vector size proportional to the actual magnitude of the result.