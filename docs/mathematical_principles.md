# Mathematical principles

This document separates three ideas that are easy to conflate: truncating a
mathematical constant to 200 decimal places, truncating a fixed-point quotient,
and recording the exact integer remainder that measures the latter.

## Notation and stored constants

Let `k` denote either π or e and define

```text
S = 10^200
C = floor(k S)
delta = k - C/S
```

Then `C` is the 201-digit nonnegative integer obtained by deleting the decimal
point from the checked-in constant text, and

```text
0 <= delta < 1/S = 10^-200.
```

`delta` is the representation error introduced by retaining 200 fractional
digits. The implementation never attempts to recover the omitted irrational
tail.

The integer `C` is stored in radix `B = 10^9`:

```text
C = limb[0] + limb[1] B + ... + limb[n-1] B^(n-1).
```

The limbs are little-endian with respect to significance, not host byte order.
A 201-digit integer needs `ceil(201/9) = 23` active limbs. The public array has
24 slots, or 216 decimal digits of capacity, leaving one spare limb.

## Exact division and the two error terms

For a positive state-dependent divisor `d`, Euclidean division gives unique
integers `q` and `r` such that

```text
C = q d + r,        0 <= r < d.
```

Dividing by `d S` and substituting `k = C/S + delta` yields the exact identity

```text
k/d = q/S + r/(d S) + delta/d.
```

This identity is the precise interpretation of the public “round-off” data:

- `r/(d S)` is the fixed-point quotient truncation error relative to the
  stored integer `C`;
- `delta/d` is the earlier error from truncating the mathematical constant;
- `r` itself is an exact Euclidean remainder, not a floating-point estimate;
- `r/d` is the discarded fractional part measured in units of one fixed-point
  least-significant decimal place.

The API retains the name `roundoff` because the remainder diagnoses the part
discarded when the fixed-point quotient is rounded toward zero. It must not be
read as a claim that the unknown tail of π or e has been measured.

## Limb-wise division invariant

Long division processes limbs from most significant to least significant. If
`r_prev` is the remainder after the already processed prefix, the next step is

```text
current = r_prev B + limb[i]
q_i     = floor(current / d)
r_next  = current mod d.
```

The invariant `0 <= r_prev < d` holds initially because the first remainder is
zero. Euclidean division preserves it at every step, so the final `r_next` is
exactly `C mod d`. Computing the modulus of the full 201-digit integer in a
language with arbitrary-precision integers must therefore produce the same
answer as the C limb loop.

The intermediate is safe in `uint64_t`:

```text
current <= (d - 1) B + (B - 1) = d B - 1
        <= (2^32 - 1) 10^9 - 1
        < 2^64.
```

No 128-bit extension or floating-point environment is involved.

## Uniform bounded reduction

Let a 64-bit source value be uniform over `0 .. 2^64 - 1`, and let
`1 <= b <= 2^64 - 1`. Define

```text
t = 2^64 mod b.
```

Unsigned C evaluates `(UINT64_C(0) - b) % b` to this value because unsigned
arithmetic is modulo `2^64`. Reject values below `t`; the accepted domain has

```text
2^64 - t = floor(2^64 / b) b
```

members. Its size is an exact multiple of `b`, and reduction modulo `b` gives
every result the same number of preimages.

The rejection probability is always below one half. If `b <= 2^63`, then
`t < b <= 2^63`. If `b > 2^63`, the quotient is one and
`t = 2^64 - b < 2^63`. Thus `t/2^64 < 1/2` in either case, and the geometric
expected number of source draws is

```text
1 / (1 - t/2^64) < 2.
```

This proof establishes exact mapping uniformity conditional on uniform source
words. It does not prove that an arbitrary deterministic source is random or
unpredictable.

## Inclusive signed intervals

For `int32_t` endpoints `minimum <= maximum`, the implementation computes

```text
span = (int64_t)maximum - (int64_t)minimum + 1.
```

The mathematical span lies in `1 .. 2^32`, so it is representable in both
`int64_t` and `uint64_t`. A sampled offset lies in `0 .. span - 1`; therefore
`(int64_t)minimum + offset` lies in the original interval and is representable
as `int32_t`. For the complete domain, the calculation is exactly

```text
INT32_MAX - INT32_MIN + 1 = 2^32.
```

Promoting after a 32-bit subtraction would be too late because the subtraction
itself could already have undefined behavior.

## Deterministic 64-bit primality classification

For odd `n > 2`, write `n - 1 = 2^s d` with odd `d`. A Miller–Rabin witness
checks `a^d mod n` and at most `s - 1` repeated squares. The implementation
uses the seven-base set

```text
2, 325, 9375, 28178, 450775, 9780504, 1795265022
```

whose exhaustively established coverage reaches the complete unsigned 64-bit
domain. Each base is reduced modulo `n`; a zero base is skipped.

Portable modular multiplication uses add-and-double. At most 64 iterations are
needed per multiplication, trading speed for identical behavior on compilers
with and without a native 128-bit integer type. See [References](references.md)
for the witness record and algorithm sources.
