# Secure randomness

This document defines the security boundary of the `crng_secure_*` functions.
It is not a general-purpose cryptographic design guide.

## Threat model

The wrapper is intended to obtain unpredictable bytes when:

- the selected operating-system random facility is correctly implemented,
  initialized, and uncompromised;
- the process and caller buffer are not already exposed to the adversary;
- the caller checks the returned status and handles the output safely;
- the surrounding protocol uses random material with appropriate length and
  uniqueness requirements.

The library defends against accidentally substituting its deterministic game
generator for a native provider. It does not defend against a compromised
kernel, process-memory disclosure, rollback/snapshot duplication, malicious
build substitution, side channels in later operations, or misuse of the
returned bytes.

## Native trust boundary

`crng_secure_*` implements no entropy collector or cryptographic generator. It
delegates to:

- Windows: `BCryptGenRandom` with `BCRYPT_USE_SYSTEM_PREFERRED_RNG`;
- Apple and supported BSD systems: `arc4random_buf`;
- Linux and Android API 28 or newer: `getrandom`, with `/dev/urandom` only when
  the syscall is unavailable and reports `ENOSYS`;
- Android API 27 or earlier: the checked `/dev/urandom` read loop, because the
  Bionic `getrandom` interface is not available to those deployment targets;
- other supported POSIX systems: `/dev/urandom`.

Hardware-event collection, boot readiness, reseeding, and the underlying
cryptographic construction remain inside the OS facility. See
[References](references.md) for native documentation.

## Failure and partial-output behavior

`crng_secure_bytes(NULL, 0)` succeeds. A null pointer with nonzero size returns
`CRNG_ERR_NULL` without calling the provider.

Other native failures return `CRNG_ERR_SYSTEM`. The code retries interrupted
Linux/POSIX reads and accounts for partial successful reads, but a later call
can still fail after a buffer prefix was written. On any non-success result:

1. treat the entire requested buffer as invalid;
2. do not use the prefix that may have been produced;
3. discard or erase caller memory according to application policy;
4. abort the security-sensitive operation or propagate the error.

The library does not erase the buffer because it does not own the memory and
cannot know the caller’s erasure policy. It never replaces a provider failure
with a timestamp, PID, `rand()`, pi/e residues, or other deterministic input.

Normal CI exercises successful native calls and public input validation. The
current design has no injectable provider abstraction, so interrupted,
partial, unavailable, and failing backend branches do not yet have controlled
failure-injection tests. This is a documented testing gap, not a guarantee
inferred from the happy path.

## Deterministic seeding is not a secure stream

`crng_game_rng_seed_auto` obtains one `uint64_t` seed through
`crng_secure_u64`, then runs the public deterministic generator. It can make a
game session non-repeatable when the seed is not recorded, but it does not give
the stream forward secrecy, backtracking resistance, prediction resistance, or
a cryptographic security proof. State compromise exposes its continuation.

Use `crng_secure_bytes` directly for keys, salts, tokens, and protocol material
whose secrecy or unpredictability matters.

## Uniform secure choices

`crng_secure_uniform_u64` uses rejection sampling so every value in
`[0, upper_bound)` has the same number of accepted 64-bit preimages, conditional
on uniform native source words. `crng_secure_range_i32` extends this mapping to
inclusive signed intervals without overflow.

Modulo reduction of one byte or word is biased whenever the requested
cardinality does not divide the source-domain cardinality. Rejection removes
that mapping bias; it cannot repair a compromised source.

## Tokens, keys, salts, and nonces

The `secure_token` example requests 32 bytes and hex-encodes them. Encoding
doubles printed length without increasing entropy. The appropriate byte count
depends on the protocol and attack model.

Random bytes are only one input to a secure system:

- passwords require a reviewed password-hashing construction and parameters;
- key derivation requires an appropriate KDF and domain separation;
- nonces may require uniqueness rules stronger or different from randomness;
- secrets require access control, constant-time use where appropriate, and
  lifecycle management;
- comparisons and transport require protocol-level protection.

Use a dedicated reviewed cryptographic library for authenticated encryption,
hashing, KDFs, signatures, key exchange, and secure-memory facilities.

## Prime utility boundary

`crng_crypto_is_prime_u64` exactly classifies unsigned 64-bit inputs under the
documented witness record. `crng_crypto_random_prime_u64` repeatedly samples
uniform integer candidates from the requested interval and returns a prime if
one is found within the attempt budget.

This is an educational/number-theory utility, not cryptographic key
generation. The 64-bit domain is far too small for common public-key security,
the search does not enforce safe/strong-prime structure or protocol policy,
and no constant-time or side-channel claim is made. `CRNG_ERR_NOT_FOUND` means
only that random attempts were exhausted; it does not prove the interval
contains no prime.

## Explicit non-claims

- No FIPS or other cryptographic-module validation.
- No entropy estimate independent of the operating system.
- No memory locking, erasure, or post-return secret protection.
- No rollback, VM snapshot, or fork uniqueness protocol.
- No cryptographic claim for `crng_game_rng` or high-precision residues.
- No inference of unpredictability from statistical smoke tests.
