#ifndef GAME_RNG_H
#define GAME_RNG_H

#include <stdint.h>
#include <stdbool.h>
#include "../common/constants.h"

typedef struct {
    uint64_t digits[BIGINT_WORDS];
    int length;
} FastBigInt;

typedef struct {
    uint64_t state[4];
    FastBigInt pi;
    FastBigInt e;
    uint64_t rotation_primes[8];
} GameRNG;

void init_game_rng(GameRNG* rng);
uint64_t next_random(GameRNG* rng);
int random_range(GameRNG* rng, int min, int max);
double random_float(GameRNG* rng);

// Prime and mixing functions
bool is_prime(uint64_t n);

// BigInt operations
void bigint_init(FastBigInt *num, const char *value);

void run_benchmark(GameRNG* rng, int num_iterations);

#endif // GAME_RNG_H