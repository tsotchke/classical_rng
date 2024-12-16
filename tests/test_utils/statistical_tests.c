#include "statistical_tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define NUM_BUCKETS TEST_BUCKETS
#define NUM_SAMPLES TEST_SAMPLES

TestResults* init_test_results(void) {
    TestResults *results = malloc(sizeof(TestResults));
    if (!results) return NULL;

    results->distribution = calloc(TEST_BUCKETS, sizeof(uint64_t));
    results->bit_counts = calloc(64, sizeof(uint64_t));
    results->sequence_correlation = calloc(TEST_SEQUENCE_LENGTH, sizeof(double));
    
    if (!results->distribution || !results->bit_counts || !results->sequence_correlation) {
        free_test_results(results);
        return NULL;
    }
    
    memset(results->transition_matrix, 0, sizeof(results->transition_matrix));
    results->chi_square = 0.0;
    results->bit_entropy = 0.0;
    results->generation_time = 0.0;
    results->numbers_per_second = 0;
    
    return results;
}

void free_test_results(TestResults *results) {
    if (results) {
        free(results->distribution);
        free(results->bit_counts);
        free(results->sequence_correlation);
        free(results);
    }
}

void run_distribution_test(TestResults *results, uint64_t *values, size_t count) {
    double expected = (double)count / TEST_BUCKETS;
    results->chi_square = 0.0;
    
    for (size_t i = 0; i < count; i++) {
        size_t bucket = values[i] % TEST_BUCKETS;
        results->distribution[bucket]++;
    }
    
    for (size_t i = 0; i < TEST_BUCKETS; i++) {
        double diff = results->distribution[i] - expected;
        results->chi_square += (diff * diff) / expected;
    }
}

void run_bit_analysis(TestResults *results, uint64_t *values, size_t count) {
    uint64_t prev_bits = 0;
    double total_transitions = 0;
    
    for (size_t i = 0; i < count; i++) {
        uint64_t value = values[i];
        
        for (int bit = 0; bit < 64; bit++) {
            if (value & (1ULL << bit)) {
                results->bit_counts[bit]++;
            }
            
            if (i > 0) {
                int prev = (prev_bits >> bit) & 1;
                int curr = (value >> bit) & 1;
                results->transition_matrix[prev][curr]++;
                total_transitions++;
            }
        }
        
        prev_bits = value;
    }
    
    // Calculate bit entropy
    results->bit_entropy = 0.0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            double p = results->transition_matrix[i][j] / total_transitions;
            if (p > 0) {
                results->bit_entropy -= p * log2(p);
            }
        }
    }
}

void run_sequence_analysis(TestResults *results, uint64_t *values, size_t count) {
    for (size_t i = 1; i < TEST_SEQUENCE_LENGTH && i < count; i++) {
        double norm_prev = (double)values[i-1] / UINT64_MAX;
        double norm_curr = (double)values[i] / UINT64_MAX;
        results->sequence_correlation[i] = norm_prev - norm_curr;
    }
}

void output_json_results(const TestResults *results, const char *rng_name) {
    printf("{\n  \"rng\": \"%s\",\n", rng_name);
    
    printf("  \"distribution\": [");
    for (int i = 0; i < NUM_BUCKETS; i++) {
        printf("%llu%s", (unsigned long long)results->distribution[i], 
               i < NUM_BUCKETS-1 ? "," : "");
    }
    printf("],\n");
    
    printf("  \"bit_counts\": [");
    for (int i = 0; i < 64; i++) {
        printf("%llu%s", (unsigned long long)results->bit_counts[i], 
               i < 63 ? "," : "");
    }
    printf("],\n");
    
    printf("  \"transition_matrix\": [[%llu,%llu],[%llu,%llu]],\n",
           (unsigned long long)results->transition_matrix[0][0], 
           (unsigned long long)results->transition_matrix[0][1],
           (unsigned long long)results->transition_matrix[1][0], 
           (unsigned long long)results->transition_matrix[1][1]);
    
    printf("  \"metrics\": {\n");
    printf("    \"chi_square\": %f,\n", results->chi_square);
    printf("    \"bit_entropy\": %f,\n", results->bit_entropy);
    printf("    \"generation_time\": %f,\n", results->generation_time);
    printf("    \"numbers_per_second\": %llu\n", 
           (unsigned long long)results->numbers_per_second);
    printf("  }\n}\n");
}
