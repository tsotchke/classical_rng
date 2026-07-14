# classical_rng

`classical_rng` is a C11 random-number library built around the project’s
original two-part architecture:

- `game_rng` is a reproducible generator whose output incorporates exact
  roundoff residues from 200-fractional-digit fixed-point representations of
  pi and e.
- `crypto_rng` obtains secret randomness from the operating system and retains
  the project’s random-prime/number-theory layer with portable 64-bit
  arithmetic.

Version 2 makes the high-precision constant machinery operational and
observable, fixes the integer-range crash reported in
[PR #1](https://github.com/tsotchke/classical_rng/pull/1), consistently
namespaces the public API, and adds portable builds for GCC, Clang, and MSVC.
It preserves the recognizable game/crypto split and the constant-based
architecture rather than replacing it with a generic PRNG wrapper.

## What the constant engine does

For either mathematical constant `k`, define `S = 10^200` and
`C = floor(kS)`. The game generator stores `C` as a nonnegative big integer in
base `10^9`. On every draw it:

1. advances a xoshiro256** state seeded through SplitMix64;
2. derives state-dependent 32-bit divisors;
3. divides both high-precision constants limb by limb;
4. captures exact Euclidean remainders discarded by fixed-point quotient
   truncation;
5. avalanches those residues together with the core output.

For seed `1`, the first pi calculation is exactly:

```text
remainder / divisor = 3488475904 / 3772506329
```

The remainder is not a truncated `double`; all 200 fractional digits
participate in exact integer division. If `C = qd + r` and
`k = C/S + delta`, then

```text
k/d = q/S + r/(dS) + delta/d,    0 <= r < d, 0 <= delta < 10^-200.
```

Thus `r/(dS)` is the fixed-precision quotient truncation term, while
`delta/d` is the separate error introduced by storing only 200 fractional
digits. The API retains “roundoff” for the former, exact and observable,
quantity. Use `crng_game_rng_last_roundoff` or `crng_constant_roundoff` to
inspect each step.

This construction is useful for reproducible generation and for studying
fixed-point error propagation. It is a custom noncryptographic output
composition: the project proves the integer arithmetic and bounded mappings,
but does not claim a least scalar-output period, an equidistribution dimension,
independent nearby-seed streams, or cryptographic unpredictability. Because pi
and e are public constants, their residues are not secret entropy.

## Choose the right module

| Task | API | Property |
|---|---|---|
| Reproducible simulation or game | `crng_game_rng_*` | Same v2 algorithm, seed, and call schedule give the same outputs |
| Procedural generation | `crng_game_rng_*` | Explicit state and unbiased bounded mapping |
| Inspect high-precision roundoff | `crng_game_rng_last_roundoff` | Exact quotient remainders for pi and e |
| Token, salt, key, or nonce | `crng_secure_*` | Native OS cryptographic randomness |
| Random 64-bit prime | `crng_crypto_random_prime_u64` | OS candidate selection and deterministic Miller-Rabin |

## Quick start

```c
#include <classical_rng.h>
#include <inttypes.h>
#include <stdio.h>

int main(void) {
    crng_game_rng rng;
    crng_roundoff_sample roundoff;
    uint64_t die;

    crng_game_rng_seed(&rng, UINT64_C(2026));
    if (crng_game_rng_uniform_u64(&rng, UINT64_C(6), &die) != CRNG_OK) {
        return 1;
    }
    if (crng_game_rng_last_roundoff(&rng, &roundoff) != CRNG_OK) {
        return 1;
    }

    printf("d6 = %" PRIu64 ", pi remainder = %" PRIu32 "\n",
           die + UINT64_C(1), roundoff.pi.remainder);
    return 0;
}
```

Build and test with CMake:

```sh
cmake -S . -B build \
  -DCRNG_BUILD_TESTS=ON \
  -DCRNG_BUILD_EXAMPLES=ON \
  -DCRNG_WARNINGS_AS_ERRORS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Or use Make on Unix-like systems and MinGW:

```sh
make test
make examples
./build/crng_constant_roundoff
./build/game_rng_cli --seed 1 --count 4 --show-roundoff
./build/crypto_rng_cli --bytes 32
```

## Repository architecture

```text
include/
  classical_rng.h                 umbrella header
  classical_rng/
    common.h                      status, version, visibility
    game_rng.h                    high-precision deterministic engine
    crypto_rng.h                  OS randomness and prime generation
src/
  common/                         shared constants and status text
  game_rng/                       fixed-point residues, generator, CLI
  crypto_rng/                     OS backends, number theory, CLI
examples/
  game/                           particles and terrain
  crypto/                         tokens and KDF salt preparation
docs/                             theory, implementation, API, exercises
tests/                            vectors, boundaries, portability checks
```

Legacy include paths under `src/game_rng` and `src/crypto_rng` remain as
compatibility facades. Define `CRNG_ENABLE_V1_COMPAT` before including one to
enable the original unnamespaced spellings for staged migration. New code
should use installed headers and `crng_`/`CRNG_` identifiers.

## Correct range handling

Bounded functions use rejection sampling rather than plain modulo. Signed
inclusive ranges calculate their span after promoting both endpoints to
`int64_t`, so `INT32_MIN..INT32_MAX` correctly has `2^32` outcomes. Reversed
ranges return `CRNG_ERR_INVALID_RANGE` rather than reaching a zero divisor.

## Secure platform backends

| Platform | Source |
|---|---|
| Windows | `BCryptGenRandom` |
| macOS and supported BSDs | `arc4random_buf` |
| Linux and Android API 28+ | `getrandom`, with `/dev/urandom` fallback for `ENOSYS` |
| Android API 27 and earlier | `/dev/urandom` |
| Other POSIX systems | `/dev/urandom` |

The library has no global mutable RNG state and no heap allocation. Native
entropy failures are returned rather than hidden behind the deterministic
generator. A caller must treat the requested buffer as unusable after failure
because a prefix may already have been filled.

## Documentation

- [Documentation index](docs/README.md)
- [Getting started](docs/getting-started.md)
- [Complete API reference](docs/api-reference.md)
- [Guarantees, evidence, and non-guarantees](docs/guarantees.md)
- [Deterministic generator specification](docs/deterministic-prng.md)
- [Mathematical principles](docs/mathematical_principles.md)
- [Implementation details](docs/implementation_details.md)
- [Secure-randomness threat boundary](docs/secure-randomness.md)
- [Bit distribution and test interpretation](docs/bit_distribution.md)
- [Performance analysis](docs/performance_analysis.md)
- [Portability and integration](docs/portability.md)
- [Example applications](docs/example_applications.md)
- [Hands-on learning guide](docs/learning-guide.md)
- [Primary references and provenance](docs/references.md)
- [Contributing](CONTRIBUTING.md)
- [Changelog](CHANGELOG.md)

## License

MIT. See [LICENSE](LICENSE). The xoshiro256** and SplitMix64 algorithms were
published by their cited authors; this repository contains its own C11
implementation and fixed-point constant-residue composition. See
[References](docs/references.md).
