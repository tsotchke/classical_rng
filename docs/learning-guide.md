# Hands-on learning guide

These exercises connect implementation, proof, and empirical evidence. Fixed
seeds make observations reproducible; they do not make empirical results
universal.

## 1. Reproduce a versioned call schedule

```sh
make examples
./build/crng_dice 1
./build/crng_dice 1
./build/crng_dice 2
```

The first two runs match and the third differs. Record the seed and
`CRNG_VERSION_STRING`. Then compare eight `next_u64` calls with:

- one `fill` request of `8 * sizeof(uint64_t)`;
- eight `fill` requests of `sizeof(uint64_t)`;
- two `fill` requests of three and five bytes.

Expected conclusion: the first two fill schedules agree byte-for-byte, while
partial calls consume complete words and make the third schedule diverge.
Reproducibility is a property of version, seed, and call schedule together.

## 2. Derive rejection sampling on a toy domain

List source integers `0` through `15` and map each with `x % 6`. Buckets zero
through three have three preimages; four and five have two. Compute
`t = 16 mod 6 = 4`, reject `x < 4`, and map the remaining twelve values.

Expected conclusion: every output now has exactly two accepted preimages. Then
repeat the cardinality proof symbolically for `2^64` and bound `b`. Explain why
the proof is conditional on uniform source words and does not prove the
deterministic generator itself is random.

## 3. Prove the complete signed range

```c
int32_t value;
crng_status status = crng_game_rng_range_i32(
    &rng, INT32_MIN, INT32_MAX, &value
);
```

Evaluate the span first in signed 32-bit arithmetic, then as:

```text
(int64_t)INT32_MAX - (int64_t)INT32_MIN + 1 = 2^32.
```

Expected conclusion: casting after subtraction is too late because the
subtraction has already overflowed. Show that adding any accepted offset in
`0..2^32-1` to `INT32_MIN` remains representable in `int64_t` and maps back
into the exact `int32_t` domain.

## 4. Verify limb division against a big integer

Run `crng_constant_roundoff` and obtain the first pi divisor. Remove the decimal
point from `crng_game_rng_constant_decimal("pi")` and interpret the result as
one arbitrary-precision integer `C`. Compute `C mod d`.

For seed 1 after the first draw:

```text
d = 3772506329
C mod d = 3488475904.
```

Now implement toy long division in base 1000 and verify that its final
remainder equals big-integer modulus. Expected conclusion: the quotient can be
discarded while the limb invariant still establishes the exact full-integer
remainder.

## 5. Separate both approximation terms

Let `S = 10^200`, `C = floor(pi S)`, and `C = qd + r`. Derive:

```text
pi/d = q/S + r/(dS) + delta/d
delta = pi - C/S.
```

Classify `r/(dS)` and `delta/d`. Expected conclusion: the public remainder
measures fixed-precision quotient truncation exactly, while the omitted
irrational tail is a distinct bounded representation error. Neither value is
floating-point noise.

## 6. Reconstruct one output equation

From [deterministic-prng.md](deterministic-prng.md), implement one version
2.0.0 step in a language with explicit 64-bit unsigned wraparound. Use seed 1
and compare:

- raw xoshiro output;
- post-transition state and step;
- both derived divisors;
- both exact remainders;
- mixed residue;
- final output `0xbdfd01ff6a8c2511`.

Expected conclusion: the constant layer changes returned output but not the
xoshiro state recurrence. Identify which statements concern internal state
period and which would require a separate proof about scalar output.

## 7. Shuffle correctly

Read [`examples/shuffle.c`](../examples/shuffle.c). Fisher-Yates chooses among
exactly the unshuffled positions. Prove by induction that every permutation has
probability `1/n!` conditional on unbiased bounded draws.

Compare this with sorting by a random comparator. Expected conclusion: a random
comparator may violate the sorting contract and does not generally define a
uniform distribution over permutations.

## 8. Audit empirical evidence

Read [bit_distribution.md](bit_distribution.md) and the test sources. Record
which measurements are merely printed and which actually gate success. Then
design a TestU01 or PractRand experiment with a predeclared sample plan,
multiple-testing policy, output encoding, and retained seeds.

Expected conclusion: known-answer tests establish stream identity more directly
than histograms, while statistical suites can expose defects but do not prove
unpredictability or entropy provenance.

## 9. Exercise dynamic and cross-language checks

```sh
make sanitize
```

Also configure the CMake suite so the C++ header test and installed-package
consumer run. Expected conclusion: AddressSanitizer and
UndefinedBehaviorSanitizer provide evidence only for executed paths; a clean
header compile provides source-integration evidence, not binary ABI stability.

## 10. Design entropy failure injection

Without changing the public API, sketch an internal provider abstraction that
can simulate `EINTR`, a short read, `ENOSYS`, a late failure after a prefix, and
a Windows provider error. Define observable postconditions for each case.

Expected conclusion: successful native calls do not exercise fail-closed
branches. Any future test hook must remain internal so production callers
cannot replace the OS provider accidentally.
