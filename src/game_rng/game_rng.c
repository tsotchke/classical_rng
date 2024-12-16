#include "game_rng.h"
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// [Keep all existing functions but remove the main() function and print_usage()]
bool is_prime(uint64_t n) {
    if (n <= 3) return n > 1;
    if (!(n & 1) || n % 3 == 0) return false;
    
    uint64_t limit = sqrt(n);
    for (uint64_t i = 5; i <= limit; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

static uint64_t find_next_prime(uint64_t n) {
    while (true) {
        if (n % 2 == 0) n++;
        if (is_prime(n)) return n;
        n += 2;
    }
}

static void generate_rotation_primes(GameRNG* rng) {
    uint64_t candidate = MIN_ROTATION;
    for (int i = 0; i < NUM_ROTATION_PRIMES; i++) {
        uint64_t prime = find_next_prime(candidate);
        while (prime > MAX_ROTATION) {
            candidate = MIN_ROTATION;
            prime = find_next_prime(candidate);
        }
        rng->rotation_primes[i] = prime;
        candidate = prime + 1;
    }
}

static inline uint64_t rotate_left(uint64_t x, uint64_t r) {
    r = r % WORD_SIZE_BITS;
    return (x << r) | (x >> (WORD_SIZE_BITS - r));
}

static inline uint64_t mix_fast(uint64_t x, uint64_t y, GameRNG* rng) {
    for (int stage = 0; stage < NUM_MIXING_STAGES; stage++) {
        x = rotate_left(x, rng->rotation_primes[stage*2]);
        y = rotate_left(y, rng->rotation_primes[stage*2 + 1]);
        
        x *= (stage % 2 ? CONSTANT_PHI : CONSTANT_E);
        y *= (stage % 2 ? CONSTANT_PI : CONSTANT_ROOT2);
        
        uint64_t temp = x;
        x = y ^ rotate_left(x, rng->rotation_primes[(stage*2 + 2) % NUM_ROTATION_PRIMES]);
        y = temp ^ rotate_left(y, rng->rotation_primes[(stage*2 + 3) % NUM_ROTATION_PRIMES]);
    }
    
    return x ^ y;
}

void bigint_init(FastBigInt *num, const char *value) {
    num->length = (strlen(value) + 7) / 8;
    for (int i = 0; i < num->length; i++) {
        num->digits[i] = value[i] ? value[i] - '0' : 0;
    }
}

void init_game_rng(GameRNG* rng) {
    bigint_init(&rng->pi, "31415926535897932384626433832795");
    bigint_init(&rng->e, "27182818284590452353602874713526");
    
    rng->state[0] = CONSTANT_PHI;
    rng->state[1] = CONSTANT_E;
    rng->state[2] = CONSTANT_PI;
    rng->state[3] = CONSTANT_ROOT2;
    
    generate_rotation_primes(rng);
}

uint64_t next_random(GameRNG* rng) {
    uint64_t mixed = mix_fast(rng->state[0], rng->state[1], rng);
    mixed = mix_fast(mixed, rng->state[2], rng);
    mixed = mix_fast(mixed, rng->state[3], rng);
    
    uint64_t temp = rng->state[0];
    rng->state[0] = mix_fast(rng->state[1], rng->rotation_primes[0], rng);
    rng->state[1] = mix_fast(rng->state[2], rng->rotation_primes[1], rng);
    rng->state[2] = mix_fast(rng->state[3], rng->rotation_primes[2], rng);
    rng->state[3] = mix_fast(temp, rng->rotation_primes[3], rng);
    
    return mixed;
}

int random_range(GameRNG* rng, int min, int max) {
    uint64_t range = (uint64_t)(max - min + 1);
    return min + (int)(next_random(rng) % range);
}

double random_float(GameRNG* rng) {
    return (next_random(rng) >> 11) * 0x1.0p-53;
}

void run_benchmark(GameRNG* rng, int num_iterations) {
    clock_t start = clock();
    
    volatile uint64_t dummy = 0;
    for (int i = 0; i < num_iterations; i++) {
        dummy ^= next_random(rng);
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Generated %d random numbers in %.3f seconds\n", num_iterations, time_spent);
    printf("Speed: %.2f million numbers per second\n", num_iterations / (time_spent * 1000000));
}