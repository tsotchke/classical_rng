# The deterministic game generator

The deterministic subsystem composes a published 256-bit state transition with
the project’s original high-precision constant-remainder construction. The
composition is intentionally inspectable:

```text
64-bit seed
    |
    v
SplitMix64 expansion --> four 64-bit state words
    |
    v
xoshiro256** raw word + state transition + step counter
    |
    +--> state-dependent pi/e divisors
    |        |
    |        v
    |    exact C mod d for both 200-digit constants
    |        |
    +--------+--> output mixing --> returned 64-bit word
```

The constant layer changes the returned stream; it is not unused diagnostic
state. It does not feed back into the four-word xoshiro transition.

## Fixed-point inputs

For each constant `k`, the implementation stores `C = floor(k 10^200)` in 23
active base-`10^9` limbs. The parser and exact division algorithm are derived
in [Mathematical principles](mathematical_principles.md). The checked-in text,
not a platform math library, is the canonical input.

For each divisor `d`, the division computes the quotient only as workspace and
retains `r = C mod d`. The quotient is not mixed into the output. The public
`crng_roundoff_component` exposes `d` and `r`, so another implementation can
verify the calculation using arbitrary-precision integer modulus.

## Seed expansion and transition

One seed is expanded into four words by four successive SplitMix64 calls. Let
all operations below be unsigned modulo `2^64`, and define

```text
Kphi = 0x9e3779b97f4a7c15
K1   = 0xbf58476d1ce4e5b9
K2   = 0x94d049bb133111eb
K3   = 0x6a09e667f3bcc909
```

The private constant names preserve version-1 nomenclature; `K1`, `K2`, and
`K3` are bit-mixing constants, not fixed-point encodings of e, π, or sqrt(2).

A SplitMix64 expansion step is

```text
x = x + Kphi
z = x
z = (z xor (z >> 30)) * K1
z = (z xor (z >> 27)) * K2
z = z xor (z >> 31).
```

For state `(s0, s1, s2, s3)`, the xoshiro256** raw output is

```text
raw = rotl64(s1 * 5, 7) * 9.
```

The implementation then applies the published XOR/shift/rotate transition.
All operands are unsigned; the rotation counts are fixed in `1 .. 63`, so the
C expressions avoid invalid-width shifts.

## Divisor derivation and output construction

Let the primed state be the state after the xoshiro transition, and let `n` be
the incremented 64-bit step counter. Define

```text
fold32(x) = low32(x) xor high32(x)
odd32(x)  = max(fold32(x) or 1, 3)
A(x)      = x xor (x >> 30)
            then multiply by K1
            then xor with (x >> 27)
            then multiply by K2
            then xor with (x >> 31)
```

More precisely, each “then” in `A` uses the result of the preceding line. The
two divisors are

```text
d_pi = odd32(raw xor s0' xor n)
d_e  = odd32(rotl64(raw, 29) xor s3' xor (n * K1)).
```

They are always odd and at least three. Oddness is a deterministic design
choice, not a claimed primality or statistical guarantee.

With `r_pi = C_pi mod d_pi` and `r_e = C_e mod d_e`, the recorded mixed value
and returned word are

```text
z = (r_pi << 32) or r_e
z = z xor (d_pi << 17)
z = z xor rotl64(d_e, 43)
m = A(z)

output = A(raw xor m xor (n * K3)).
```

These equations, the canonical constant strings, and the known-answer vectors
fully specify version 2.0.0’s deterministic stream. “Avalanche” in source names
describes the intended role of `A`; this project does not claim a formal or
empirical avalanche criterion for the composed generator.

## Period statements and non-statements

The published xoshiro256** transition over a nonzero four-word state has period
`2^256 - 1`. The separate 64-bit step counter wraps after `2^64` increments.
Because these periods are coprime, the pair `(xoshiro state, step)` has internal
recurrence period `2^64 (2^256 - 1)` for a nonzero xoshiro state.

That is a statement about internal state recurrence, not a proof of the least
period of the scalar returned sequence. The output function is not proven
injective over internal states, and no least-output-period, equidistribution,
cryptographic distinguishability, or prediction-resistance claim is made for
the custom composition.

## Reproducibility and streams

For version 2.0.0, the same seed and call sequence produce the same numeric
outputs and diagnostic residues on every supported platform. Record
`CRNG_VERSION_STRING`, the seed, and the sequence of API calls with scientific
or simulation results.

The project does not currently expose xoshiro jump functions or a stream-split
API. Different or nearby seeds are useful for distinct reproducible runs, but
are not a formal proof of independent substreams. Copying a seeded
`crng_game_rng` value within one program creates an identical continuation;
persistent binary serialization and cross-version layout compatibility are not
supported.

## Bounded integers and binary64 values

Bounded integer APIs use rejection sampling. The proof of uniform mapping and
the full signed-range proof are in
[Mathematical principles](mathematical_principles.md).

`crng_game_rng_next_double` takes the upper 53 bits `k` of one output and
returns exactly the binary64 value

```text
k * 2^-53,       0 <= k < 2^53.
```

The result set has spacing `2^-53`, includes zero, and excludes one. This is a
mapping contract; its probability statement remains conditional on the source
word distribution.

## Intended use

The generator is appropriate for reproducible games, procedural generation,
teaching, and experiments with fixed-point remainder propagation. It is
predictable by design. Public constants, reversible state evolution, and a
custom output mixer do not create secret entropy. Use the OS-backed API for
security-sensitive randomness.
