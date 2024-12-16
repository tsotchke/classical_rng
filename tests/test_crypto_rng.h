#ifndef TEST_CRYPTO_RNG_H
#define TEST_CRYPTO_RNG_H

#include <stdint.h>
#include <stdbool.h>
#include "statistical_tests.h"
#include "../src/crypto_rng/crypto_rng.h"
#include "../src/common/constants.h"

// Test configuration
void run_crypto_rng_tests(void);
void run_crypto_rng_benchmark(int num_iterations);
TestResults* perform_crypto_rng_analysis(uint64_t num_samples);

#endif // TEST_CRYPTO_RNG_H