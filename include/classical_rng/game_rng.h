#ifndef CLASSICAL_RNG_GAME_RNG_H
#define CLASSICAL_RNG_GAME_RNG_H

#include "classical_rng/common.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CRNG_HIGH_PRECISION_LIMBS 24

/**
 * Nonnegative fixed-point decimal stored in little-endian base-10^9 limbs.
 * `fractional_digits` gives the implicit decimal scale. Fields are public for
 * inspection and education; callers must not mutate a seeded generator's copy.
 */
typedef struct crng_high_precision_decimal {
    uint32_t limbs[CRNG_HIGH_PRECISION_LIMBS];
    size_t used;
    size_t fractional_digits;
} crng_high_precision_decimal;

/** One exact Euclidean remainder `r` satisfying `C = q * divisor + r`. */
typedef struct crng_roundoff_component {
    uint32_t divisor;
    uint32_t remainder;
} crng_roundoff_component;

/**
 * Diagnostic record for the high-precision constant step behind one output.
 * `mixed_residue` is the avalanche of both divisor/remainder pairs; it is not
 * entropy and must not be treated as a cryptographic value.
 */
typedef struct crng_roundoff_sample {
    uint64_t step;
    crng_roundoff_component pi;
    crng_roundoff_component e;
    uint64_t mixed_residue;
} crng_roundoff_sample;

/**
 * State for the deterministic game/simulation generator.
 *
 * Keep one state per stream. Concurrent access to the same object requires
 * synchronization; different objects may be used concurrently. The layout is
 * public for inspection, but direct mutation and cross-version serialization
 * are outside the API contract.
 */
typedef struct crng_game_rng {
    uint64_t state[4];
    uint64_t step;
    crng_high_precision_decimal pi;
    crng_high_precision_decimal e;
    crng_roundoff_sample last_roundoff;
} crng_game_rng;

/**
 * Return static canonical 200-fractional-digit text for "pi" or "e".
 * Return NULL for a null or unknown name. The result must not be freed.
 */
CRNG_API const char *crng_game_rng_constant_decimal(const char *name);

/**
 * Copy the exact fixed-point division residues used for the latest output.
 * A freshly seeded generator reports an all-zero sample until its first draw.
 */
CRNG_API crng_status crng_game_rng_last_roundoff(
    const crng_game_rng *rng,
    crng_roundoff_sample *out
);

/**
 * Initialize a deterministic game/simulation stream from a 64-bit seed.
 * Every seed value is valid. A null pointer is ignored.
 */
CRNG_API void crng_game_rng_seed(crng_game_rng *rng, uint64_t seed);

/**
 * Initialize a game/simulation stream from one OS-random 64-bit seed.
 * The resulting stream remains a noncryptographic deterministic stream.
 */
CRNG_API crng_status crng_game_rng_seed_auto(crng_game_rng *rng);

/**
 * Generate the next 64 bits from a seeded deterministic stream.
 * `rng` must point to an object initialized by a seed function.
 */
CRNG_API uint64_t crng_game_rng_next_u64(crng_game_rng *rng);

/** Generate the upper 32 bits of the next 64-bit stream output. */
CRNG_API uint32_t crng_game_rng_next_u32(crng_game_rng *rng);

/**
 * Fill a buffer from a deterministic stream in stable little-endian order.
 * A null buffer is valid only when size is zero. This is not cryptographic.
 */
CRNG_API crng_status crng_game_rng_fill(
    crng_game_rng *rng,
    void *buffer,
    size_t size
);

/**
 * Draw uniformly from [0, upper_bound) by rejection sampling.
 * A zero bound is invalid. Write `out` only on success.
 */
CRNG_API crng_status crng_game_rng_uniform_u64(
    crng_game_rng *rng,
    uint64_t upper_bound,
    uint64_t *out
);

/**
 * Draw uniformly from inclusive [minimum, maximum] without signed overflow.
 * Reversed endpoints are invalid. Write `out` only on success.
 */
CRNG_API crng_status crng_game_rng_range_i32(
    crng_game_rng *rng,
    int32_t minimum,
    int32_t maximum,
    int32_t *out
);

/**
 * Generate `k / 2^53` in [0, 1), using the upper 53 output bits.
 * `rng` must point to a seeded object.
 */
CRNG_API double crng_game_rng_next_double(crng_game_rng *rng);

#ifdef __cplusplus
}
#endif

#endif /* CLASSICAL_RNG_GAME_RNG_H */
