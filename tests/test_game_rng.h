#ifndef TEST_GAME_RNG_H
#define TEST_GAME_RNG_H

#include <stdint.h>
#include <stdbool.h>
#include "statistical_tests.h"
#include "../src/game_rng/game_rng.h"
#include "../src/common/constants.h"

// Test configuration
void run_game_rng_tests(void);
void run_game_rng_benchmark(int num_iterations);
TestResults* perform_game_rng_analysis(uint64_t num_samples);

#endif // TEST_GAME_RNG_H