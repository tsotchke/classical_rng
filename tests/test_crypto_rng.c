#include "classical_rng/crypto_rng.h"
#include "test_crypto_rng.h"
#include "test_utils/statistical_tests.h"

#include <stdlib.h>

int run_crypto_rng_tests(void) {
    enum { SAMPLE_COUNT = 4096 };
    crng_test_distribution report;
    uint64_t *values;
    uint64_t prime = 0;
    size_t index;
    int result = 0;

    values = (uint64_t *)malloc(sizeof(*values) * SAMPLE_COUNT);
    if (values == NULL) {
        return 1;
    }
    for (index = 0; index < SAMPLE_COUNT; ++index) {
        if (crng_secure_u64(&values[index]) != CRNG_OK) {
            free(values);
            return 1;
        }
    }

    crng_test_analyze_distribution(&report, values, SAMPLE_COUNT);
    if (!crng_test_distribution_has_coverage(&report) ||
        crng_crypto_random_prime_u64(17, 17, 1, &prime) != CRNG_OK ||
        prime != UINT64_C(17)) {
        result = 1;
    }
    crng_test_print_json("crypto_rng", &report);
    free(values);
    return result;
}

int main(void) {
    return run_crypto_rng_tests();
}
