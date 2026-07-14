#ifndef CLASSICAL_RNG_V1_CRYPTO_COMPAT_H
#define CLASSICAL_RNG_V1_CRYPTO_COMPAT_H

#include "classical_rng/crypto_rng.h"

/* Opt-in source compatibility for the original crypto_rng header. */
#if defined(CRNG_ENABLE_V1_COMPAT)
typedef struct EntropyState {
    uint64_t counter;
    uint64_t timestamp;
    uint64_t mixer[4];
} EntropyState;

typedef struct BigInt {
    uint64_t words[8];
    size_t used_words;
} BigInt;

static inline uint64_t secure_random(
    uint64_t prime_lower,
    uint64_t prime_upper,
    int rounds
) {
    uint64_t result = 0;
    (void)prime_lower;
    (void)prime_upper;
    (void)rounds;
    (void)crng_secure_u64(&result);
    return result;
}

static inline bool is_prime(uint64_t value, int rounds) {
    (void)rounds;
    return crng_crypto_is_prime_u64(value);
}

static inline uint64_t generate_random_prime(
    uint64_t lower_bound,
    uint64_t upper_bound,
    int rounds
) {
    uint64_t result = 0;
    uint64_t attempts = rounds > 0 ? (uint64_t)rounds * UINT64_C(64) : UINT64_C(64);
    (void)crng_crypto_random_prime_u64(
        lower_bound,
        upper_bound,
        attempts,
        &result
    );
    return result;
}
#endif

#endif /* CLASSICAL_RNG_V1_CRYPTO_COMPAT_H */
