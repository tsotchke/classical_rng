#include "statistical_tests.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

void crng_test_analyze_distribution(
    crng_test_distribution *report,
    const uint64_t *values,
    size_t count
) {
    size_t index;

    memset(report, 0, sizeof(*report));
    report->sample_count = count;
    for (index = 0; index < count; ++index) {
        unsigned int bit;
        ++report->buckets[values[index] & UINT64_C(15)];
        for (bit = 0; bit < 64U; ++bit) {
            report->one_bits[bit] += (values[index] >> bit) & UINT64_C(1);
        }
    }
}

int crng_test_distribution_has_coverage(const crng_test_distribution *report) {
    size_t index;

    if (report == NULL || report->sample_count < CRNG_TEST_BUCKETS) {
        return 0;
    }
    for (index = 0; index < CRNG_TEST_BUCKETS; ++index) {
        if (report->buckets[index] == 0) {
            return 0;
        }
    }
    return 1;
}

void crng_test_print_json(
    const char *generator,
    const crng_test_distribution *report
) {
    size_t index;

    printf("{\"generator\":\"%s\",\"samples\":%zu,\"buckets\":[",
           generator,
           report->sample_count);
    for (index = 0; index < CRNG_TEST_BUCKETS; ++index) {
        printf("%s%" PRIu64, index == 0 ? "" : ",", report->buckets[index]);
    }
    printf("],\"one_bits\":[");
    for (index = 0; index < 64U; ++index) {
        printf("%s%" PRIu64, index == 0 ? "" : ",", report->one_bits[index]);
    }
    printf("]}\n");
}
