#ifndef CLASSICAL_RNG_STATISTICAL_TESTS_H
#define CLASSICAL_RNG_STATISTICAL_TESTS_H

#include <stddef.h>
#include <stdint.h>

#define CRNG_TEST_BUCKETS 16

typedef struct crng_test_distribution {
    uint64_t buckets[CRNG_TEST_BUCKETS];
    uint64_t one_bits[64];
    size_t sample_count;
} crng_test_distribution;

void crng_test_analyze_distribution(
    crng_test_distribution *report,
    const uint64_t *values,
    size_t count
);

int crng_test_distribution_has_coverage(const crng_test_distribution *report);

void crng_test_print_json(
    const char *generator,
    const crng_test_distribution *report
);

#endif /* CLASSICAL_RNG_STATISTICAL_TESTS_H */
