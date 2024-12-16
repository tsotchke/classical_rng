#include "crypto_rng.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

static __thread EntropyState entropy_state = {
    .counter = 0,
    .timestamp = 0,
    .mixer = {CONSTANT_PHI, CONSTANT_E, CONSTANT_PI, CONSTANT_ROOT2}
};

static inline uint64_t rotate_left(uint64_t x, unsigned int n) {
    return (x << n) | (x >> (WORD_SIZE_BITS - n));
}

static uint64_t secure_mix(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3) {
    for (int i = 0; i < DEFAULT_MIXING_ROUNDS; i++) {
        // Round 1: Full-width mixing with prime-based rotations
        v0 = rotate_left(v0, MIN_ROTATION);
        v1 ^= v0;
        v1 = rotate_left(v1, NUM_MIXING_STAGES + MIN_ROTATION);
        v0 += v1;
        
        // Round 2: Cross-mixing with different rotations
        v2 = rotate_left(v2, NUM_ROTATION_PRIMES + MIN_ROTATION);
        v3 ^= v2;
        v3 = rotate_left(v3, NUM_MIXING_STAGES * 2);
        v2 += v3;
        
        // Round 3: Diagonal mixing
        v0 ^= v3;
        v1 ^= v2;
        
        // Round 4: Column mixing with mathematical constants
        v0 *= CONSTANT_PHI;
        v1 *= CONSTANT_E;
        v2 *= CONSTANT_PI;
        v3 *= CONSTANT_ROOT2;
    }
    
    return v0 ^ v1 ^ v2 ^ v3;
}

static void bigint_init(BigInt *num, const char *value) {
    memset(num->words, 0, sizeof(num->words));
    size_t len = strlen(value);
    num->used_words = (len + BIGINT_WORD_BITS - 1) / BIGINT_WORD_BITS;
    
    for (size_t i = 0; i < len; i++) {
        size_t word_idx = i / BIGINT_WORD_BITS;
        size_t bit_pos = (i % BIGINT_WORD_BITS) * 4;
        num->words[word_idx] |= ((uint64_t)(value[len - i - 1] - '0')) << bit_pos;
    }
}

static void bigint_mul(BigInt *result, const BigInt *a, uint64_t b) {
    uint64_t carry = 0;
    for (size_t i = 0; i < BIGINT_WORDS; i++) {
        __uint128_t prod = (__uint128_t)a->words[i] * b + carry;
        result->words[i] = (uint64_t)prod;
        carry = (uint64_t)(prod >> BIGINT_WORD_BITS);
    }
    result->used_words = a->used_words + (carry > 0 ? 1 : 0);
}

static void bigint_div(BigInt *result, const BigInt *a, uint64_t b) {
    uint64_t remainder = 0;
    result->used_words = a->used_words;
    
    for (size_t i = a->used_words; i > 0; i--) {
        __uint128_t current = ((__uint128_t)remainder << BIGINT_WORD_BITS) + a->words[i-1];
        result->words[i-1] = (uint64_t)(current / b);
        remainder = (uint64_t)(current % b);
    }
    
    while (result->used_words > 0 && result->words[result->used_words - 1] == 0) {
        result->used_words--;
    }
}

bool is_prime(uint64_t n, int rounds) {
    if (n <= 3) return n > 1;
    if (!(n & 1) || n % 3 == 0) return false;
    
    // Trial division for small primes
    for (uint64_t i = 5; i * i <= n && i <= 100; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    
    // Miller-Rabin test
    uint64_t d = n - 1;
    int r = 0;
    while (!(d & 1)) {
        d >>= 1;
        r++;
    }
    
    static const uint64_t bases[] = {2, 3, 5, 7, 11};
    int num_bases = sizeof(bases) / sizeof(bases[0]);
    
    for (int i = 0; i < num_bases && i < rounds; i++) {
        uint64_t a = bases[i];
        if (a >= n-2) continue;
        
        uint64_t x = 1;
        uint64_t exp = d;
        
        while (exp) {
            if (exp & 1)
                x = ((__uint128_t)x * a) % n;
            a = ((__uint128_t)a * a) % n;
            exp >>= 1;
        }
        
        if (x == 1 || x == n-1) continue;
        
        bool composite = true;
        for (int j = 0; j < r-1; j++) {
            x = ((__uint128_t)x * x) % n;
            if (x == n-1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

uint64_t generate_random_prime(uint64_t lower_bound, uint64_t upper_bound, int rounds) {
    while (true) {
        entropy_state.counter++;
        uint64_t candidate = lower_bound + (secure_mix(
            entropy_state.mixer[0] ^ entropy_state.counter,
            entropy_state.mixer[1] ^ (entropy_state.counter >> 16),
            entropy_state.mixer[2] ^ (entropy_state.counter << 16),
            entropy_state.mixer[3]) % (upper_bound - lower_bound + 1));
            
        if (candidate % 2 == 0) candidate++;
        if (is_prime(candidate, rounds)) return candidate;
    }
}

static uint64_t generate_entropy(uint64_t prime_lower, uint64_t prime_upper, int rounds) {
    static const char PI_DIGITS[] = "31415926535897932384626433832795028841971693993751";
    static const char E_DIGITS[] = "27182818284590452353602874713526624977572470936999";
    
    BigInt pi, e, result;
    bigint_init(&pi, PI_DIGITS);
    bigint_init(&e, E_DIGITS);
    
    uint64_t prime1 = generate_random_prime(prime_lower, prime_upper, rounds);
    uint64_t prime2 = generate_random_prime(prime_lower, prime_upper, rounds);
    
    BigInt temp;
    bigint_mul(&temp, &pi, prime1);
    bigint_div(&result, &temp, prime2);
    
    entropy_state.mixer[0] ^= prime1;
    entropy_state.mixer[1] ^= prime2;
    entropy_state.mixer[2] ^= result.words[0];
    entropy_state.mixer[3] = secure_mix(
        entropy_state.mixer[0],
        entropy_state.mixer[1],
        entropy_state.mixer[2],
        entropy_state.mixer[3]
    );
    
    return entropy_state.mixer[3];
}

uint64_t secure_random(uint64_t prime_lower, uint64_t prime_upper, int rounds) {

    entropy_state.counter++;
    entropy_state.timestamp ^= entropy_state.counter;
    
    for (int i = 0; i < NUM_MIXING_STAGES; i++) {
        entropy_state.mixer[i] ^= generate_entropy(prime_lower, prime_upper, rounds);
        entropy_state.mixer[i] = secure_mix(
            entropy_state.mixer[0],
            entropy_state.mixer[1],
            entropy_state.mixer[2],
            entropy_state.mixer[3]
        );
    }

    return entropy_state.mixer[3];
}

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [OPTIONS]\n\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --num_values N        Number of random values to generate (default: 10)\n");
    fprintf(stderr, "  --prime_lower N       Lower bound for prime numbers (default: 2^64)\n");
    fprintf(stderr, "  --prime_upper N       Upper bound for prime numbers (default: 2^65)\n");
    fprintf(stderr, "  --mixing_rounds N     Number of mixing rounds (default: %d)\n", DEFAULT_MIXING_ROUNDS);
    fprintf(stderr, "  --format FORMAT       Output format: decimal, hex (default: decimal)\n");
    fprintf(stderr, "  --benchmark           Run performance benchmark\n");
    fprintf(stderr, "  --benchmark_size N    Number of iterations for benchmark (default: 10000000)\n");
    fprintf(stderr, "  --quiet              Suppress additional output\n");
    fprintf(stderr, "  --help               Display this help message\n");
}
