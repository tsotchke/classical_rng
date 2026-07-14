# Getting started

## Requirements and assumptions

- A C11 compiler: GCC, Clang, AppleClang, or MSVC.
- Exact-width `uint32_t` and `uint64_t` types, 8-bit bytes, and a
  `double` with at least 53 significant bits. The public header checks the
  byte and floating-point requirements at compile time.
- CMake 3.16 or newer for the portable build.
- Optional: Make on Unix-like systems or MinGW.

No third-party runtime library is required. Windows links the system `bcrypt`
library for `BCryptGenRandom`. Public headers may be included from C++11 or
newer and apply `extern "C"` to the C ABI.

## Build the full validation target

```sh
cmake -S . -B build \
  -DCRNG_BUILD_TESTS=ON \
  -DCRNG_BUILD_EXAMPLES=ON \
  -DCRNG_WARNINGS_AS_ERRORS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

For multi-configuration generators, `--config Release` selects the actual
configuration. Relevant options are:

| Option | Default | Meaning |
|---|---:|---|
| `BUILD_SHARED_LIBS` | `OFF` | Build a shared instead of static library. |
| `CRNG_BUILD_EXAMPLES` | `ON` | Build numerical, game, and crypto examples. |
| `CRNG_BUILD_TOOLS` | `ON` | Build both command-line tools. |
| `CRNG_BUILD_TESTS` | follows `BUILD_TESTING` | Build unit, module, compatibility, and C++-header tests. |
| `CRNG_WARNINGS_AS_ERRORS` | `OFF` | Promote the platform warning policy to a gate. |
| `CRNG_ENABLE_SANITIZERS` | `OFF` | Enable ASan and UBSan under GCC or Clang. |

The secondary Make workflow builds a static library:

```sh
make test
make examples
make sanitize
```

## Consume the library with CMake

As a source dependency:

```cmake
add_subdirectory(external/classical_rng)
target_link_libraries(my_program PRIVATE classical_rng::classical_rng)
```

Or install and discover the exported package:

```sh
cmake --install build --prefix "$HOME/.local"
```

```cmake
find_package(classical_rng 2 CONFIG REQUIRED)
target_link_libraries(my_program PRIVATE classical_rng::classical_rng)
```

The package uses same-major compatibility. That constrains package discovery,
not deterministic stream compatibility: reproducible artifacts should record
`CRNG_VERSION_STRING`, the seed, and the API call schedule.

## Reproducible deterministic values

```c
#include <classical_rng.h>
#include <inttypes.h>
#include <stdio.h>

int main(void) {
    crng_game_rng rng;
    uint64_t index;
    crng_status status;

    crng_game_rng_seed(&rng, UINT64_C(123));
    status = crng_game_rng_uniform_u64(&rng, UINT64_C(10), &index);
    if (status != CRNG_OK) {
        fprintf(stderr, "%s\n", crng_status_string(status));
        return 1;
    }
    printf("%" PRIu64 "\n", index);
    return 0;
}
```

For version 2.0.0, the same explicit seed and sequence of calls reconstruct the
same numeric outputs, fixed-point remainders, and documented fill-byte order on
supported platforms. Bounded draws can consume multiple source words, and a
partial `fill` call consumes a complete final word, so a seed alone is
insufficient if the call schedule changes.

`crng_game_rng_seed_auto` obtains one seed from the OS and then initializes the
same deterministic, noncryptographic generator. It is suitable for an
unrepeatable game session, not for generating secrets.

## Secret random bytes

```c
unsigned char key[32];
crng_status status = crng_secure_bytes(key, sizeof(key));
if (status != CRNG_OK) {
    fprintf(stderr, "randomness failed: %s\n", crng_status_string(status));
    return 1;
}
```

Treat source failure as terminal for the operation. A failing call may have
written a prefix of a large buffer, so discard or securely erase the entire
caller-owned buffer according to application policy. Never fall back to a
timestamp, process ID, `rand()`, or the deterministic generator for security
material.

## Inspect the central numerical mechanism

```sh
./build/crng_constant_roundoff
./build/game_rng_cli --seed 1 --count 4 --show-roundoff
./build/crypto_rng_cli --bytes 32
```

The first two commands expose exact `C mod d` values for the
200-fractional-digit constants. Continue with the
[API contract](api-reference.md),
[mathematical derivation](mathematical_principles.md), and
[guarantee boundary](guarantees.md).
