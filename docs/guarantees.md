# Guarantees, evidence, and non-guarantees

This page is the normative claim boundary for version 2.0.0. Other documents
explain mechanisms; this page states what callers may rely on and what the
project has not established.

## Claim matrix

| Claim | Status | Evidence |
|---|---|---|
| Same seed and API call sequence give the same deterministic outputs on supported platforms | Guaranteed for version 2.0.0 | Known-answer vectors on AppleClang, GCC, and MSVC |
| π/e division records satisfy `C = qd + r`, `0 <= r < d` | Exact by construction | Euclidean limb invariant and known-answer remainders |
| Bounded mapping is uniform if source words are uniform | Proven conditional claim | Accepted-domain cardinality proof |
| Full `int32_t` inclusive range is free of signed overflow | Proven for the implementation types | `int64_t` span proof, boundary regression, UBSan |
| Deterministic primality classification covers every `uint64_t` | Guaranteed by the selected witness record | Seven fixed witnesses plus regression cases |
| OS byte API delegates to the documented native facility and never falls back to the game generator | Implementation contract | Backend inspection and successful-path platform tests |
| Linux/POSIX entropy-source failures preserve status and output contracts | Release-qualified with an external failure-injection harness | `EINTR`, partial reads, `ENOSYS` fallback, open failure, zero read, late `EIO`, and scalar/prime propagation were exercised; the harness is not yet a checked-in public test |
| Native entropy-source failure is propagated correctly on every non-POSIX backend | Implemented; not failure-injection qualified on every native OS | Static review plus successful-path platform tests |
| The custom deterministic output has a particular least period or equidistribution dimension | No claim | Output injectivity and distribution have not been proven |
| The deterministic generator is cryptographically unpredictable | Explicitly false | State and constants are public and deterministic |
| Nearby seeds create independent parallel streams | No claim | No split/jump API or independence proof |
| Raw struct bytes are a stable serialization | No claim | Public layout may change across versions and architectures |
| Smoke histograms validate randomness or security | Explicitly false | They only detect some gross regressions |
| The OS wrapper is a validated cryptographic module | No claim | Security remains within the host facility and integration |

## Reproducibility scope

Reproducibility requires all of the following to match:

- library version and deterministic algorithm revision;
- seed;
- order and kind of calls, because bounded rejection can consume multiple
  source words and `fill` consumes whole 64-bit words even for a partial tail;
- any copied or restored state used by the application.

Numeric outputs and `crng_game_rng_fill` byte streams are architecture-stable
for version 2.0.0. The in-memory bytes of `crng_game_rng` are not a portable
file format. Store the seed and replayable call schedule, or define an
application-level serialization tied to an exact library version.

## Concurrency

There is no global mutable RNG state. Different `crng_game_rng` objects may be
used concurrently. A single object is mutable and requires external
synchronization if shared. Read-only constant lookup, status text, and primality
classification are reentrant. OS-random calls rely on the documented
thread-safety of their native facilities.

## Security boundary

The deterministic generator is outside the security boundary even when seeded
by `crng_game_rng_seed_auto`: after seeding, its stream is deterministic and
state compromise reveals its continuation.

The secure functions are thin, fail-closed delegates. Their security is no
greater than the selected OS facility, the caller’s error handling, and the
subsequent handling of returned memory. The library does not estimate entropy,
provide forward secrecy, lock or erase memory, manage nonces, derive keys, or
validate higher-level protocols.
