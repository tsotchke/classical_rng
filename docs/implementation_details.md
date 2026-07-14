# Implementation details

This page maps the mathematical specification to the C11 implementation. It is
descriptive of version 2.0.0; public caller obligations are normative in the
[API reference](api-reference.md).

## Module boundaries

| Component | Responsibility |
|---|---|
| `src/game_rng/game_rng.c` | Parse canonical constants, perform exact base-`10^9` division, advance deterministic state, mix residues, and implement bounded game values. |
| `src/crypto_rng/crypto_rng.c` | Select the native entropy backend, implement bounded secure values, and provide portable 64-bit primality operations. |
| `src/common/constants.h` | Hold private unsigned mixing constants shared by seed expansion and avalanche functions. |
| `src/common/status.c` | Map common status values to immutable diagnostic text. |
| `include/classical_rng/*.h` | Define the installed, namespaced C/C++ source interface and visibility controls. |

The deterministic module calls the crypto module only in
`crng_game_rng_seed_auto`. Ordinary seeded generation has no system calls.
The crypto module never calls the deterministic module.

## Fixed-point representation

Canonical pi and e strings contain one integral digit and 200 fractional
digits. Removing the decimal point gives a 201-digit integer, which needs
`ceil(201/9) = 23` active base-`10^9` limbs. The public array has 24 slots.
Limbs are stored least-significant first:

```text
C = limb[0] + limb[1] B + ... + limb[used-1] B^(used-1), B = 10^9.
```

Parsing processes decimal digits left-to-right with multiply-by-ten and carry.
Before each update, `limb < B` and `carry < 10`, so
`limb * 10 + carry < 10^10` and fits in `uint64_t`.

## Exact small-divisor division

Division walks active limbs from most to least significant. At each step:

```text
current = previous_remainder * B + limb[i]
quotient[i] = current / divisor
next_remainder = current % divisor.
```

The divisor is a nonzero `uint32_t`. Since
`previous_remainder < divisor <= 2^32 - 1`:

```text
current <= (2^32 - 2) * 10^9 + (10^9 - 1) < 2^64.
```

The quotient is transient workspace and is intentionally discarded after each
constant division. The exact final remainder enters `crng_roundoff_sample` and
the output mixer. No floating-point type, `libm` routine, compiler 128-bit
extension, or host byte order participates.

## Deterministic step ordering

One `crng_game_rng_next_u64` call:

1. computes xoshiro256** raw output from the pre-transition state;
2. applies the xoshiro state transition;
3. increments the 64-bit step counter;
4. derives two divisors from the raw word, post-transition state, and step;
5. computes `C_pi mod d_pi` and `C_e mod d_e` exactly;
6. records both divisor/remainder pairs and their mixed residue;
7. returns an avalanche of raw output, mixed residue, and step.

The precise equations are in [deterministic-prng.md](deterministic-prng.md).
This order is locked by known-answer vectors. Reordering state transition,
step increment, or divisor derivation changes the versioned stream.

The constant residues affect returned output but do not feed back into the
four-word xoshiro transition. Consequently, the xoshiro state evolution can be
analyzed separately, while the scalar output properties of the composition
still require their own evidence.

## Byte and floating mappings

`crng_game_rng_fill` serializes each generated word from least- to
most-significant byte. It consumes a full word for a partial final chunk and
discards unused high bytes. This makes each individual call portable, while
also making the call schedule part of reproducibility.

`crng_game_rng_next_double` converts the upper 53 bits to
`k * 2^-53`. On supported mainstream targets, binary64 `double` represents
every one of these grid points exactly. This is a 53-bit discrete mapping, not
arbitrary real precision.

## Unbiased range mapping

Both deterministic and secure bounded APIs compute:

```c
threshold = (UINT64_C(0) - upper_bound) % upper_bound;
```

Unsigned wraparound makes this `2^64 mod upper_bound`. Values below the
threshold are rejected, leaving a domain whose cardinality is an exact
multiple of the bound. Invalid inputs are checked before state or entropy is
consumed.

Signed interval width is calculated after converting both `int32_t` endpoints
to `int64_t`. The offset is added in `int64_t` as well. This directly fixes the
overflow/divide-by-zero path reported in PR #1, including the full
`INT32_MIN..INT32_MAX` domain.

## Native entropy loops

- Windows splits requests at `ULONG_MAX` and checks every
  `BCryptGenRandom` status.
- Linux and Android API 28+ retry `getrandom` after `EINTR` and use
  `/dev/urandom` only when the syscall reports `ENOSYS`.
- Android API 27 and earlier use the checked `/dev/urandom` path directly;
  Bionic does not expose `getrandom` to those deployment targets.
- Generic POSIX code opens `/dev/urandom` with `O_CLOEXEC` where available,
  caps individual reads, and handles interruptions and short reads.
- Apple and supported BSD targets delegate the complete request to
  `arc4random_buf`.

A multi-call request can fail after writing a prefix. The API reports failure
without a fallback, and callers must discard the complete requested output.

## Portable primality arithmetic

Miller-Rabin modular multiplication uses add-and-double rather than
`__uint128_t`. The helper maintains operands reduced modulo `n` and evaluates
addition as either `left + right` or an equivalent subtraction branch chosen
before unsigned overflow. At most 64 multiplier bits are processed.

This favors a common, inspectable implementation across GCC, Clang, and MSVC
over platform-specific peak speed. The seven-witness coverage and algorithm
are documented in [mathematical_principles.md](mathematical_principles.md).

## Allocation, state, and failure model

Library code performs no heap allocation and owns no global mutable RNG state.
Canonical strings and status text are immutable static data. A
`crng_game_rng` owns its entire stream state; separate objects are reentrant,
while sharing one mutable object requires caller synchronization.

Status-returning scalar APIs write outputs only on success. Secure byte filling
is the deliberate exception because native APIs can make progress before
failure. Void/numeric deterministic draw functions require a seeded non-null
object; this precondition avoids a branch and status channel on every draw.
