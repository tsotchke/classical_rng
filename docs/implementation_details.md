# Implementation Details

This document provides a comprehensive overview of the implementation details for both RNG variants, including code structure, design decisions, and internal mechanisms.

## Code Architecture

### Project Structure
```
src/
├── common/          # Shared utilities and constants
├── game_rng/        # Fast game-oriented RNG
└── crypto_rng/      # Secure cryptographic RNG
```

### Common Components

1. **Constants and Configurations**
```c
// src/common/constants.h
#define BIGINT_WORDS 8
#define MIN_PRIME_BITS 32
#define DEFAULT_MIXING_ROUNDS 20
```

2. **Shared Types**
```c
typedef struct {
    uint64_t words[BIGINT_WORDS];
    size_t used_words;
} BigInt;
```

## Game RNG Implementation

### State Management

1. **RNG State Structure**
```c
typedef struct {
    uint64_t state[4];        // Current state vector
    FastBigInt pi;           // Pi digits for mixing
    FastBigInt e;            // e digits for mixing
    uint64_t rotation_primes[8]; // Prime numbers for rotation
} GameRNG;
```

2. **Initialization Process**
```c
void init_game_rng(GameRNG* rng) {
    // Initialize state vector with entropy sources
    for (int i = 0; i < 4; i++) {
        rng->state[i] = get_system_entropy();
    }
    
    // Initialize mathematical constants
    bigint_init(&rng->pi, PI_DIGITS);
    bigint_init(&rng->e, E_DIGITS);
    
    // Set up rotation primes
    generate_rotation_primes(rng->rotation_primes);
}
```

### Core Generation Algorithm

1. **Next Random Value**
```c
uint64_t next_random(GameRNG* rng) {
    // Apply rotation mixing
    for (int i = 0; i < 4; i++) {
        rng->state[i] = rotate_left(
            rng->state[i], 
            rng->rotation_primes[i]
        );
    }
    
    // Mix with mathematical constants
    mix_with_constants(rng);
    
    // Combine state elements
    return combine_state(rng->state);
}
```

2. **Mixing Function**
```c
static void mix_with_constants(GameRNG* rng) {
    // XOR with pi digits
    for (int i = 0; i < 4; i++) {
        rng->state[i] ^= rng->pi.digits[i];
    }
    
    // Rotate based on e digits
    for (int i = 0; i < 4; i++) {
        uint8_t rotation = rng->e.digits[i] & 0x3F;
        rng->state[i] = rotate_left(rng->state[i], rotation);
    }
}
```

## Crypto RNG Implementation

### Entropy Management

1. **Entropy Pool**
```c
typedef struct {
    uint64_t counter;
    uint64_t timestamp;
    uint64_t mixer[4];
} EntropyState;

static void update_entropy_pool(EntropyState* state) {
    // Mix in new system entropy
    state->mixer[0] ^= get_cpu_entropy();
    state->mixer[1] ^= get_timer_entropy();
    state->mixer[2] ^= get_system_entropy();
    state->mixer[3] ^= ++state->counter;
    
    // Update timestamp
    state->timestamp = get_high_precision_time();
}
```

2. **Entropy Collection**
```c
static uint64_t collect_entropy(void) {
    uint64_t entropy = 0;
    
    // Collect from multiple sources
    entropy ^= read_cpu_timestamp();
    entropy ^= get_system_entropy();
    entropy ^= get_process_metrics();
    
    return entropy;
}
```

### Secure Random Generation

1. **Main Generation Function**
```c
uint64_t secure_random(uint64_t prime_lower, 
                      uint64_t prime_upper, 
                      int rounds) {
    uint64_t result = 0;
    EntropyState state;
    
    // Initialize state
    init_entropy_state(&state);
    
    // Apply mixing rounds
    for (int i = 0; i < rounds; i++) {
        // Mix entropy
        update_entropy_pool(&state);
        
        // Generate intermediate value
        result ^= generate_intermediate(&state);
        
        // Apply cryptographic transformation
        result = apply_crypto_transform(result);
    }
    
    // Ensure output is within range
    return scale_to_range(result, prime_lower, prime_upper);
}
```

2. **Prime Number Generation**
```c
uint64_t generate_random_prime(uint64_t lower_bound, 
                             uint64_t upper_bound, 
                             int rounds) {
    uint64_t candidate;
    
    do {
        // Generate random number in range
        candidate = secure_random(lower_bound, upper_bound, rounds);
        
        // Ensure it's odd
        candidate |= 1;
        
        // Test primality
    } while (!is_prime(candidate, PRIME_TEST_ROUNDS));
    
    return candidate;
}
```

## Optimization Techniques

### SIMD Optimizations

1. **Vector Operations**
```c
#ifdef __AVX2__
static void vector_mix_states(__m256i* state_vec) {
    // Load rotation constants
    __m256i rot_vec = _mm256_load_si256(rotation_primes);
    
    // Perform vectorized rotation
    __m256i rotated = _mm256_sllv_epi64(*state_vec, rot_vec);
    
    // Store result
    _mm256_store_si256(state_vec, rotated);
}
#endif
```

2. **Cache Alignment**
```c
typedef struct __attribute__((aligned(64))) {
    uint64_t state[4];
    // ... other members
} GameRNG;
```

### Memory Management

1. **Stack Usage**
```c
// Avoid heap allocations in critical paths
static inline uint64_t generate_intermediate(const uint64_t* state) {
    uint64_t temp[4];  // Stack allocation
    
    // Perform operations using stack memory
    for (int i = 0; i < 4; i++) {
        temp[i] = rotate_left(state[i], i + 1);
    }
    
    return combine_values(temp);
}
```

2. **Cache-Friendly Access**
```c
// Ensure sequential memory access
static void update_state(uint64_t* state) {
    for (int i = 0; i < 4; i++) {
        // Sequential access pattern
        state[i] = transform_value(state[i]);
    }
}
```

## Testing Infrastructure

### Unit Tests

1. **Test Framework**
```c
// tests/test_game_rng.c
void test_distribution(void) {
    GameRNG rng;
    init_game_rng(&rng);
    
    // Generate large sample
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        uint64_t value = next_random(&rng);
        update_distribution_stats(value);
    }
    
    // Verify distribution properties
    assert_chi_square_within_bounds();
    assert_bit_entropy_acceptable();
}
```

2. **Statistical Tests**
```c
// tests/test_utils/statistical_tests.c
void run_full_test_suite(RNGInterface* rng) {
    TestResults results;
    
    // Run all tests
    test_distribution(&results, rng);
    test_bit_patterns(&results, rng);
    test_sequences(&results, rng);
    
    // Output results
    output_test_results(&results);
}
```

## References

1. "Efficient Implementation of Random Number Generators" - Cryptography Research Journal
2. Intel® 64 and IA-32 Architectures Software Developer's Manual
3. "Fast and Reliable Random Number Generation in C++" - ACM Computing Surveys
4. NIST Special Publication 800-90A Rev. 1
