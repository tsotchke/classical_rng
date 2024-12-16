#ifndef STATISTICAL_TESTS_H
#define STATISTICAL_TESTS_H

#include <stdint.h>
#include <stddef.h>
#include "../src/common/constants.h"

typedef struct {
    uint64_t *distribution;
    uint64_t *bit_counts;
    double *sequence_correlation;
    uint64_t transition_matrix[2][2];
    double chi_square;
    double bit_entropy;
    double generation_time;
    uint64_t numbers_per_second;
} TestResults;

TestResults* init_test_results(void);
void free_test_results(TestResults *results);
void run_distribution_test(TestResults *results, uint64_t *values, size_t count);
void run_bit_analysis(TestResults *results, uint64_t *values, size_t count);
void run_sequence_analysis(TestResults *results, uint64_t *values, size_t count);
void output_json_results(const TestResults *results, const char *rng_name);

#endif // STATISTICAL_TESTS_H