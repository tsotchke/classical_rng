# Example applications

The examples are executable demonstrations of one contract each. They are
deliberately not application frameworks or cryptographic protocols.

## Numerical introspection

[`examples/constant_roundoff.c`](../examples/constant_roundoff.c) prints the
fixed-point scale, pi/e divisor/remainder pairs, mixed residue, and final output
for four steps.

- Preconditions: a conforming build and explicit seed.
- Demonstrates: deterministic `C mod d` introspection and versioned output.
- Does not demonstrate: measurement of the omitted irrational tail,
  cryptographic entropy, or statistical quality.

## Games and simulations

| Example | Demonstrates | Deliberately omits |
|---|---|---|
| [`examples/dice.c`](../examples/dice.c) | Explicit seed and unbiased six-way mapping | Stream persistence and game architecture |
| [`examples/shuffle.c`](../examples/shuffle.c) | Fisher-Yates with unbiased shrinking bounds | Generic containers and error recovery |
| [`examples/game/particle_system.c`](../examples/game/particle_system.c) | Reproducible signed velocities | Physical realism and parallel stream splitting |
| [`examples/game/terrain_generation.c`](../examples/game/terrain_generation.c) | Deterministic procedural grid | Production terrain algorithms and serialization |

For scientific or replay use, record the library version, seed, and sequence of
calls. Merely reusing a seed after changing call order does not reproduce the
same stream position.

## Security-sensitive inputs

| Example | Demonstrates | Deliberately omits |
|---|---|---|
| [`examples/secure_token.c`](../examples/secure_token.c) | Direct 256-bit OS request, status check, hex encoding | Storage, comparison, transport, and erasure |
| [`examples/crypto/token_generation.c`](../examples/crypto/token_generation.c) | Native token generation in the legacy example family | Complete authentication/session protocol |
| [`examples/crypto/key_derivation.c`](../examples/crypto/key_derivation.c) | Native salt preparation and explicit handoff boundary | An actual KDF implementation |

The key-derivation example intentionally directs callers to reviewed Argon2id,
scrypt, PBKDF2, or HKDF implementations as appropriate. Random salt generation
does not itself derive or protect a key.

## Command-line tools

```sh
game_rng_cli --seed 1 --count 4 --show-roundoff
crypto_rng_cli --bytes 32
crypto_rng_cli --prime 1000000 2000000 --attempts 1024
```

The game tool is deterministic when its seed and options are fixed. The crypto
tool is not reproducible and exits on provider failure. Its 64-bit prime mode
is for number-theory exploration, not public-key generation.

Command output is human-facing and is not a stable machine protocol. Link the
library API for applications that need structured error handling or binary
data.
