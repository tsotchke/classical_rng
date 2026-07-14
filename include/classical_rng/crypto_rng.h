#ifndef CLASSICAL_RNG_CRYPTO_RNG_H
#define CLASSICAL_RNG_CRYPTO_RNG_H

#include "classical_rng/common.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Fill a buffer with randomness from the native operating-system CSPRNG.
 * `(NULL, 0)` succeeds. Source failures are reported without fallback.
 */
CRNG_API crng_status crng_secure_bytes(void *buffer, size_t size);

/** Generate a native-endian 64-bit value from operating-system randomness. */
CRNG_API crng_status crng_secure_u64(uint64_t *out);

/**
 * Draw uniformly from [0, upper_bound) using OS randomness and rejection.
 * A zero bound is invalid. Write `out` only on success.
 */
CRNG_API crng_status crng_secure_uniform_u64(
    uint64_t upper_bound,
    uint64_t *out
);

/**
 * Draw uniformly from inclusive [minimum, maximum] using OS randomness.
 * The entire int32_t domain is supported. Write `out` only on success.
 */
CRNG_API crng_status crng_secure_range_i32(
    int32_t minimum,
    int32_t maximum,
    int32_t *out
);

/**
 * Deterministic Miller-Rabin classification valid over all uint64_t values.
 * This is a number-theory utility, not a large-key prime generator.
 */
CRNG_API bool crng_crypto_is_prime_u64(uint64_t value);

/**
 * Search for a prime in inclusive [minimum, maximum] from uniform OS-random
 * candidates. Values below two are excluded. Returns CRNG_ERR_NOT_FOUND after
 * max_attempts nonprime candidates; that status does not prove the interval is
 * prime-free. Write `out` only on success.
 */
CRNG_API crng_status crng_crypto_random_prime_u64(
    uint64_t minimum,
    uint64_t maximum,
    uint64_t max_attempts,
    uint64_t *out
);

#ifdef __cplusplus
}
#endif

#endif /* CLASSICAL_RNG_CRYPTO_RNG_H */
