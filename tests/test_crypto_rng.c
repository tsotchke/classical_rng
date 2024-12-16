#include "test_crypto_rng.h"
#include "statistical_tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    printf("Running Cryptographic RNG tests...\n");
    
    uint64_t *values = malloc(TEST_SAMPLES * sizeof(uint64_t));
    if (!values) {
        fprintf(stderr, "Failed to allocate memory for test values\n");
        return 1;
    }
    
    TestResults *results = init_test_results();
    if (!results) {
        fprintf(stderr, "Failed to initialize test results\n");
        free(values);
        return 1;
    }
    
    clock_t start = clock();
    
    // Use much smaller values for testing
    uint64_t prime_lower = 1000000;
    uint64_t prime_upper = 2000000;
    
    for (size_t i = 0; i < TEST_SAMPLES; i++) {
        values[i] = secure_random(prime_lower, prime_upper, DEFAULT_MIXING_ROUNDS);
    }
    
    clock_t end = clock();
    results->generation_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    results->numbers_per_second = (uint64_t)(TEST_SAMPLES / results->generation_time);
    
    run_distribution_test(results, values, TEST_SAMPLES);
    run_bit_analysis(results, values, TEST_SAMPLES);
    run_sequence_analysis(results, values, TEST_SAMPLES);
    
    output_json_results(results, "crypto_rng");
    
    free(values);
    free_test_results(results);
    
    return 0;
}