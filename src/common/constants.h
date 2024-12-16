#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

#define WORD_SIZE_BITS      64
#define MIN_ROTATION        7
#define MAX_ROTATION        (WORD_SIZE_BITS - MIN_ROTATION)
#define NUM_MIXING_STAGES   4
#define NUM_ROTATION_PRIMES 8

#define CONSTANT_PHI        0x9E3779B97F4A7C15ULL
#define CONSTANT_E          0x8B8B45C25F25EA19ULL
#define CONSTANT_PI         0x243F6A8885A308D3ULL
#define CONSTANT_ROOT2      0x5F876A9349D12EA7ULL
#define CONSTANT_EULER      0x2C5C8E5B3BF4C93AULL

// Adjusted prime bits to prevent overflow
#define SECURITY_BITS          256
#define MIN_PRIME_BITS         63  // Changed from 64 to prevent shift overflow
#define DEFAULT_MIXING_ROUNDS  20
#define BIGINT_WORD_BITS       64
#define BIGINT_WORDS           (SECURITY_BITS / BIGINT_WORD_BITS)
#define MAX_DIGITS             (SECURITY_BITS / 3)

#define TEST_SAMPLES           1000000
#define TEST_BUCKETS           1000
#define TEST_SEQUENCE_LENGTH   1000

#endif // CONSTANTS_H