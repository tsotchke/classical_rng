# API reference

This page is the contract-level reference for the version 2 public API. The
mathematics of the deterministic engine is developed in
[deterministic-prng.md](deterministic-prng.md) and
[mathematical_principles.md](mathematical_principles.md). Claims and non-claims
are collected in [guarantees.md](guarantees.md).

## Headers, language, and ABI

~~~c
#include <classical_rng.h>             /* complete public API */
#include <classical_rng/common.h>      /* status vocabulary */
#include <classical_rng/game_rng.h>    /* deterministic generator */
#include <classical_rng/crypto_rng.h>  /* native OS entropy and primes */
~~~

The headers support C11 and C++ compilation. Public declarations use exact-width
integer types and require 8-bit bytes plus at least 53 significant bits in
`double`; the common header checks both assumptions. Export decoration is controlled by
CRNG_API for Windows shared-library builds; consumers must not define
CRNG_BUILDING_LIBRARY.

Version 2 does not promise a stable binary ABI across future major versions.
Source and stream compatibility are version-scoped as described below.

### Version and representation macros

| Macro | Version 2.0.0 value or role |
|---|---|
| `CRNG_VERSION_MAJOR` | `2` |
| `CRNG_VERSION_MINOR` | `0` |
| `CRNG_VERSION_PATCH` | `0` |
| `CRNG_VERSION_STRING` | `"2.0.0"` |
| `CRNG_HIGH_PRECISION_LIMBS` | `24` base-`10^9` storage slots |

Record the complete string with reproducible artifacts. The numeric macros are
compile-time interface metadata; they are not a runtime ABI-negotiation
mechanism.

## Status values

| Value | Meaning |
|---|---|
| CRNG_OK | The operation completed and documented outputs are valid. |
| CRNG_ERR_NULL | A required object, buffer, or output pointer was null. |
| CRNG_ERR_INVALID_RANGE | Bounds were empty, reversed, or otherwise invalid. |
| CRNG_ERR_SYSTEM | The native entropy provider failed. |
| CRNG_ERR_UNSUPPORTED | No supported secure-random backend was compiled. |
| CRNG_ERR_NOT_FOUND | A bounded search exhausted its attempt budget. |

### crng_status_string

~~~c
const char *crng_status_string(crng_status status);
~~~

Returns a pointer to static, immutable text. Every enum value above has a stable
description; an unrecognized integer value maps to a generic unknown-status
description. The result is never null and must not be freed.

## Public structures

### crng_high_precision_decimal

An inspectable nonnegative fixed-point integer. `limbs` is a
least-significant-first array in mathematical radix `10^9`; `used` is the
active prefix length, and `fractional_digits` is the implicit decimal scale.
The canonical constants use 23 of the 24 available slots and a scale of 200.

The type is public because `crng_game_rng` owns both parsed constants and
requires no hidden allocation. Callers may inspect a seeded object but must not
mutate these fields. Raw bytes, padding, and cross-version layout are not a
serialization format.

### crng_roundoff_component and crng_roundoff_sample

`crng_roundoff_component` is one divisor/remainder pair. Its `remainder`
satisfies `C = q * divisor + remainder` and
`0 <= remainder < divisor` for the associated stored constant.

| Member | Interpretation |
|---|---|
| step | Zero-based deterministic draw index that produced the sample. |
| pi_divisor, e_divisor | Positive 32-bit divisors derived from generator state. |
| pi_remainder, e_remainder | Exact Euclidean remainders of the stored scaled integers. |
| mixed_residue | The 64-bit value formed and diffused from both remainder channels. |

For a scaled integer C and divisor d, the recorded value is exactly
r = C mod d, with 0 <= r < d. “Roundoff” names the discarded fractional part
r/(dS) of fixed-precision quotient C/(dS); it is not a floating-point residual.

### crng_game_rng

An owning, caller-allocated deterministic stream object. It contains four
transition words, a draw counter, the parsed fixed-point pi/e integers, and the
latest roundoff sample. Its representation is public so allocation requires no
library heap, but applications should treat every member as implementation
state and initialize only through a seed function.

Copying an initialized object copies the stream position exactly. Concurrent
access to one object is a data race unless the caller synchronizes it. Separate
objects have no shared mutable library state.

## Contract matrix

“Consumes” means advances a deterministic stream or requests native entropy.
For status-returning functions, output parameters are valid only after
CRNG_OK unless a function explicitly documents partial-buffer behavior.

| Function | Preconditions and null handling | Success result | Consumption and failure |
|---|---|---|---|
| crng_status_string | Any crng_status value. | Static non-null text. | No state or entropy consumption; no failure status. |
| crng_game_rng_seed | rng may be null, in which case the call is ignored. Every 64-bit seed is valid. | Reinitializes a complete deterministic stream and clears last_roundoff. | No OS entropy; deterministic; returns void. |
| crng_game_rng_seed_auto | rng must be non-null. | Equivalent to obtaining one secure 64-bit seed and calling crng_game_rng_seed. | One native entropy request. On error, rng is not promised initialized. |
| crng_game_rng_next_u64 | rng must point to an initialized object. | Next 64 deterministic output bits. | Exactly one engine step. Invalid use is outside the contract; there is no status return. |
| crng_game_rng_next_u32 | rng must point to an initialized object. | Upper 32 bits of one next_u64 result. | Exactly one engine step. |
| crng_game_rng_fill | rng must be initialized. buffer may be null only when size is zero. | Writes size deterministic bytes in stable little-endian word order. | Consumes ceil(size/8) engine steps. Invalid null use returns CRNG_ERR_NULL without advancing. |
| crng_game_rng_uniform_u64 | rng must be initialized; out non-null; upper_bound greater than zero. | Writes a uniform integer in [0, upper_bound). | One or more engine steps due to rejection sampling. Invalid arguments do not advance; rejection count is data-dependent. |
| crng_game_rng_range_i32 | rng initialized; out non-null; minimum no greater than maximum. | Writes a uniform integer in the inclusive interval. Full int32 range is supported. | Delegates to uniform_u64 and consumes one or more steps. Invalid arguments do not advance. |
| crng_game_rng_next_double | rng must be initialized. | A binary64 value in [0,1), on implementations where double has at least 53 significant bits. | Exactly one engine step and 53 output bits. |
| crng_game_rng_constant_decimal | name must equal pi or e; null and unknown names are accepted as lookup misses. | Static canonical decimal text, or null for a miss. | No state, allocation, or failure status. |
| crng_game_rng_last_roundoff | rng and out must be non-null; rng must be initialized for meaningful state. | Copies the latest sample; it is all zero immediately after seeding. | No advancement. Null input returns CRNG_ERR_NULL and does not write out. |
| crng_secure_bytes | buffer may be null only for size zero. | Writes size bytes supplied by the native CSPRNG. | Native provider may make multiple calls. On failure, a prefix may already have been written; the remainder must be treated as unusable. |
| crng_secure_u64 | out must be non-null. | Writes one native-endian uint64 value. | One secure_bytes operation; output is valid only on CRNG_OK. |
| crng_secure_uniform_u64 | out non-null; upper_bound greater than zero. | Writes a uniform integer in [0, upper_bound). | One or more native draws due to rejection sampling. Invalid arguments make no entropy request. |
| crng_secure_range_i32 | out non-null; minimum no greater than maximum. | Writes a uniform inclusive int32 result, including over the full domain. | One or more native draws. Invalid arguments make no entropy request. |
| crng_crypto_is_prime_u64 | Any uint64 value. | Exact primality result over the full uint64 domain. | Deterministic computation only; no entropy or failure status. |
| crng_crypto_random_prime_u64 | out non-null; minimum no greater than maximum; max_attempts greater than zero. | Writes a prime sampled by repeated uniform candidates from the inclusive interval. | Each attempt may consume multiple native draws. OS errors propagate; CRNG_ERR_NOT_FOUND means only that the attempt budget expired. |

## Deterministic game API

### Seeding and stream identity

~~~c
void crng_game_rng_seed(crng_game_rng *rng, uint64_t seed);
crng_status crng_game_rng_seed_auto(crng_game_rng *rng);
~~~

For a fixed classical_rng major/minor implementation and a fixed sequence of API
calls, the same explicit seed reproduces the same results on supported
platforms. seed_auto chooses a seed with the OS CSPRNG but does not turn later
game output into cryptographic output.

No stream-splitting, jump-ahead, or statistical independence guarantee is
defined for nearby seeds or copied streams.

### Integer output and byte filling

~~~c
uint64_t crng_game_rng_next_u64(crng_game_rng *rng);
uint32_t crng_game_rng_next_u32(crng_game_rng *rng);
crng_status crng_game_rng_fill(crng_game_rng *rng, void *buffer, size_t size);
~~~

fill defines a byte representation, not a universal equivalence between call
schedules. A partial final word discards that word's unused high bytes.
Consequently, fill(rng, p, 3) followed by fill(rng, q, 5) is generally not the
same stream position or byte sequence as one fill(rng, r, 8).

### Unbiased bounded output

~~~c
crng_status crng_game_rng_uniform_u64(
    crng_game_rng *rng, uint64_t upper_bound, uint64_t *out);

crng_status crng_game_rng_range_i32(
    crng_game_rng *rng, int32_t minimum, int32_t maximum, int32_t *out);
~~~

Both functions use rejection sampling. They do not use remainder reduction
alone, which would bias results whenever the requested cardinality does not
divide 2^64. The signed-range implementation computes interval width in
uint64 arithmetic, avoiding overflow for INT32_MIN through INT32_MAX.

### Floating output

~~~c
double crng_game_rng_next_double(crng_game_rng *rng);
~~~

The mapping is (next_u64 >> 11) * 2^-53. It selects exactly 2^53 equally likely
grid points in [0,1), subject to the statistical behavior of the underlying
generator. It does not provide arbitrary real-number precision.

### Constant and roundoff introspection

~~~c
const char *crng_game_rng_constant_decimal(const char *name);
crng_status crng_game_rng_last_roundoff(
    const crng_game_rng *rng, crng_roundoff_sample *out);
~~~

The accepted names are the exact lower-case strings pi and e. Returned decimal
strings contain 200 fractional digits. The introspection sample belongs to the
most recently completed engine step and is intended for teaching, diagnostics,
and deterministic tests.

## Native secure-random API

~~~c
crng_status crng_secure_bytes(void *buffer, size_t size);
crng_status crng_secure_u64(uint64_t *out);
crng_status crng_secure_uniform_u64(uint64_t upper_bound, uint64_t *out);
crng_status crng_secure_range_i32(
    int32_t minimum, int32_t maximum, int32_t *out);
~~~

The implementation delegates to BCryptGenRandom on Windows, getrandom on Linux
and Android API 28+, `/dev/urandom` on older Android deployment targets, and
arc4random_buf on Apple and BSD targets. It fails closed: no time-based,
deterministic, or game-RNG fallback is substituted. See
[secure-randomness.md](secure-randomness.md) for the threat boundary.

## Number-theory API

~~~c
bool crng_crypto_is_prime_u64(uint64_t value);

crng_status crng_crypto_random_prime_u64(
    uint64_t minimum,
    uint64_t maximum,
    uint64_t max_attempts,
    uint64_t *out);
~~~

Primality uses deterministic Miller-Rabin with the seven-witness set documented
in [mathematical_principles.md](mathematical_principles.md). Modular
multiplication uses overflow-safe add-and-double arithmetic, avoiding a
dependency on non-standard 128-bit integer types.

Random-prime generation samples every integer candidate, not just odd values.
Conditioned on success, repeated independent uniform candidate draws give each
prime in the requested interval equal probability. Runtime is bounded by
max_attempts, and the function is not a cryptographic key-generation protocol:
it does not enforce size, safe-prime structure, secrecy policy, or side-channel
resistance.

## Version 1 source migration

The high-precision constant engine, module split, prime layer, CLI separation,
and example families remain recognizable. Version 2 replaces ambiguous global
names and unchecked return conventions:

| Version 1 spelling | Version 2 spelling |
|---|---|
| GameRNG | crng_game_rng |
| init_game_rng(&rng) | crng_game_rng_seed(&rng, seed) |
| next_random(&rng) | crng_game_rng_next_u64(&rng) |
| random_range(&rng, min, max) | crng_game_rng_range_i32(&rng, min, max, &out) |
| random_float(&rng) | crng_game_rng_next_double(&rng) |
| secure_random(...) | crng_secure_u64(&out) |
| is_prime(n, rounds) | crng_crypto_is_prime_u64(n) |
| generate_random_prime(...) | crng_crypto_random_prime_u64(...) |

For staged source migration, define CRNG_ENABLE_V1_COMPAT and include the legacy
module header. Compatibility spellings are opt-in so ordinary version 2 builds
retain a clean global namespace.
