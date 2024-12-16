#include "statistical_tests.h"
#include "test_game_rng.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    printf("Running Game RNG tests...\n");
    
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
    
    // Initialize RNG
    GameRNG rng;
    init_game_rng(&rng);
    
    // Generate test values
    clock_t start = clock();
    
    for (size_t i = 0; i < TEST_SAMPLES; i++) {
        values[i] = next_random(&rng);
    }
    
    clock_t end = clock();
    results->generation_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    results->numbers_per_second = (uint64_t)(TEST_SAMPLES / results->generation_time);
    
    // Run statistical tests
    run_distribution_test(results, values, TEST_SAMPLES);
    run_bit_analysis(results, values, TEST_SAMPLES);
    run_sequence_analysis(results, values, TEST_SAMPLES);
    
    // Output results in JSON format
    output_json_results(results, "game_rng");
    
    // Cleanup
    free(values);
    free_test_results(results);
    
    return 0;
}