# Changelog

## 2.0.0 - 2026-07-13

This release completes and hardens the original two-module architecture while
introducing a consistently namespaced API.

### Correctness

- Reimplemented the game generator around its intended high-precision constant
  design: π and e now retain 200 fractional digits in base-`10^9` limbs, exact
  division remainders feed every output, and the residue is inspectable.
- Added a xoshiro256** transition and SplitMix64 seed expansion beneath the
  high-precision residue layer.
- Connected the crypto module to direct operating-system CSPRNG access.
- Added rejection sampling for unbiased bounded integers.
- Added overflow-safe inclusive `int32_t` ranges, including the complete
  `INT32_MIN..INT32_MAX` interval.
- Added regression coverage for invalid reversed ranges and the divide-by-zero
  case reported by Richard Hoekstra in
  [PR #1](https://github.com/tsotchke/classical_rng/pull/1).
- Removed undefined signed arithmetic, nonportable `__uint128_t` use, global
  thread-local state, and rotation expressions with invalid shift counts.
- Corrected the random-prime error path so a failed native uniform draw is
  checked before its output value participates in candidate arithmetic.

### API and portability

- Namespaced every public identifier with `crng_` or `CRNG_`.
- Added native Windows, Apple/BSD, Linux, Android, and generic POSIX entropy
  backends. Android API 28+ uses `getrandom`; earlier deployment targets use
  the checked `/dev/urandom` loop so all four NDK ABIs compile from API 23.
- Added CMake install/export support, a small Makefile, and GCC/Clang/MSVC CI.
- Added Android NDK static/shared cross-build gates for API levels 23 and 28.
- Added an explicit compile-time requirement for 8-bit bytes and documented
  deterministic byte order, call-schedule consumption, C++ inclusion, and
  public-layout ABI limits.
- Preserved the random-prime layer with namespaced deterministic 64-bit
  Miller-Rabin and portable modular multiplication.
- Preserved the game/crypto CLI split and opt-in version 1 header façades.

### Education and documentation

- Rewrote the documentation around exact fixed-point round-off residues, the
  distinction between reproducibility and unpredictability, and the preserved
  game/crypto architecture.
- Added compact dice, shuffle, and secure-token examples.
- Documented modulo bias, rejection sampling, full-span arithmetic, security
  boundaries, portability, and a version 1 migration table.
- Added a complete API contract matrix, formal fixed-point error identity,
  exact deterministic-output equations, guarantee/non-guarantee matrix,
  threat model, benchmark protocol, test-evidence audit, primary references,
  and advanced exercises.

## 1.0.0 - 2024-12-16

- Initial release. Retained here only as release history; its API is not
  compatible with version 2.
