#ifndef CRYPTO_RNG_H
#define CRYPTO_RNG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common/constants.h"

typedef struct {
    uint64_t counter;
    uint64_t timestamp;
    uint64_t mixer[4];
} EntropyState;

typedef struct {
    uint64_t words[BIGINT_WORDS];
    size_t used_words;
} BigInt;

uint64_t secure_random(uint64_t prime_lower, uint64_t prime_upper, int rounds);
bool is_prime(uint64_t n, int rounds);
uint64_t generate_random_prime(uint64_t lower_bound, uint64_t upper_bound, int rounds);

#endif // CRYPTO_RNG_H