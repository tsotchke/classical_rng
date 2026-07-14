#ifndef CLASSICAL_RNG_V1_GAME_COMPAT_H
#define CLASSICAL_RNG_V1_GAME_COMPAT_H

#include "classical_rng/game_rng.h"
#include "../common/constants.h"

/*
 * The old include path remains valid. Define CRNG_ENABLE_V1_COMPAT before
 * including it to opt into the unnamespaced version 1 spellings.
 */
#if defined(CRNG_ENABLE_V1_COMPAT)
typedef crng_game_rng GameRNG;

static inline void init_game_rng(GameRNG *rng) {
    crng_game_rng_seed(rng, CRNG_V1_DEFAULT_SEED);
}

static inline uint64_t next_random(GameRNG *rng) {
    return crng_game_rng_next_u64(rng);
}

static inline int random_range(GameRNG *rng, int minimum, int maximum) {
    int32_t result = (int32_t)minimum;
    (void)crng_game_rng_range_i32(
        rng,
        (int32_t)minimum,
        (int32_t)maximum,
        &result
    );
    return (int)result;
}

static inline double random_float(GameRNG *rng) {
    return crng_game_rng_next_double(rng);
}
#endif

#endif /* CLASSICAL_RNG_V1_GAME_COMPAT_H */
