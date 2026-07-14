# Portability and integration

## Language and representation requirements

The implementation targets C11 without compiler extensions in public behavior.
It requires:

- `CHAR_BIT == 8`, enforced by the public common header;
- exact-width `uint32_t`, `uint64_t`, `int32_t`, and `int64_t` types;
- standard unsigned arithmetic modulo `2^N`;
- a `double` with at least the mainstream binary64 precision for the documented
  53-bit grid mapping.

Core integer code does not require `__uint128_t`, thread-local storage, inline
assembly, floating-point environment changes, `libm`, or heap allocation.
Fixed-width types make numeric semantics explicit; `size_t` remains
architecture-sized in public structure layout and buffer APIs.

## Supported native entropy backends

| Family | Entropy implementation | Link requirement |
|---|---|---|
| Windows Vista or newer | `BCryptGenRandom` | `bcrypt` |
| macOS | `arc4random_buf` | System libc |
| FreeBSD, OpenBSD, NetBSD, DragonFly BSD | `arc4random_buf` | System libc |
| Linux and Android API 28+ | `getrandom`; `/dev/urandom` only after `ENOSYS` | System libc |
| Android API 27 and earlier | `/dev/urandom` read loop | System libc |
| Other POSIX with `/dev/urandom` | open/read loop | System libc |

The Linux/POSIX implementation handles interrupted and partial reads. Android
selects its backend at compile time because Bionic declares `getrandom` only
from API level 28; lower deployment targets use the same checked
`/dev/urandom` loop as generic POSIX. Windows splits requests so every native
call fits `ULONG`. Native failure is returned; there is no deterministic
fallback.

## Compiler and build coverage

The warning policy is:

- GCC and Clang: `-Wall -Wextra -Wpedantic -Wconversion -Wshadow`;
- MSVC: `/W4`.

The checked-in CI matrix configures, builds, tests, installs, and compiles an
installed-package consumer with GCC and Clang on Linux, AppleClang on macOS,
and MSVC on Windows. A separate Linux Clang job runs AddressSanitizer and
UndefinedBehaviorSanitizer. An Android NDK job cross-builds static and shared
artifacts for `arm64-v8a`, `armeabi-v7a`, `x86`, and `x86_64` at API 23, plus
`arm64-v8a` at API 28 so both entropy-selection branches remain compile-gated.
CI is evidence for the revisions it executes, not a claim about every compiler
version, architecture, or downstream flag set.

CMake is authoritative for static/shared builds, installation, exports, and
the full test matrix. The Makefile is a smaller static-library workflow for
Unix-like shells and MinGW.

## Static, shared, and C++ integration

Static is the default. `-DBUILD_SHARED_LIBS=ON` enables shared builds. On
Windows, the exported target propagates `CRNG_SHARED` and the library target
privately defines `CRNG_BUILDING_LIBRARY`, selecting `dllimport` or
`dllexport` correctly.

Public declarations use `extern "C"` under C++. The project tests C++11 header
inclusion but remains a C API: it provides no C++ ownership wrappers,
exceptions, overloads, or standard-library engine concept.

Version 2 does not promise binary ABI stability across future major versions.
Public structures contain `size_t` and may have architecture-dependent padding.
Do not serialize raw structure bytes or exchange them across independently
built binaries without an application-controlled ABI agreement.

## Endianness and deterministic identity

Numeric results from `crng_game_rng_next_u64` are host-endian independent.
Fixed-point limbs encode mathematical significance, not raw byte order, and
all division uses integer values.

`crng_game_rng_fill` emits each generated word least-significant byte first,
making its per-call output stable on big- and little-endian hosts. A partial
tail still consumes a complete word, so call boundaries remain significant.
`crng_secure_bytes` returns an opaque byte string. `crng_secure_u64` loads
native bytes into a host integer; its numeric realization need not reproduce
across systems and has no reproducibility contract.

## Threading and process behavior

There is no global mutable RNG state. Different deterministic objects may be
used concurrently. One object requires external synchronization because every
draw mutates state and diagnostic fields. Native secure calls defer concurrency
behavior to their documented OS facilities.

The library defines no fork-safety protocol for an already seeded
deterministic object: after `fork`, parent and child copies continue identically
until their call schedules diverge. Reseed or otherwise separate streams when
that duplication is undesirable.

## Minimum downstream validation

```sh
cmake -S . -B build \
  -DCRNG_BUILD_TESTS=ON \
  -DCRNG_WARNINGS_AS_ERRORS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Run the sequence on every deployment toolchain. Also exercise the shared/static
mode actually shipped, compile the installed package from a clean consumer,
and run sanitizers where supported. Security-sensitive deployments should add
backend failure injection and platform-specific hardening checks.
