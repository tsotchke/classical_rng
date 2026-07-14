#include "classical_rng/game_rng.h"
#include "test_game_rng.h"
#include "test_utils/statistical_tests.h"

#include <stdlib.h>

int run_game_rng_tests(void) {
    enum { SAMPLE_COUNT = 32768 };
    crng_test_distribution report;
    crng_roundoff_sample roundoff = {0};
    crng_game_rng rng;
    uint64_t *values;
    size_t index;
    int result = 0;

    values = (uint64_t *)malloc(sizeof(*values) * SAMPLE_COUNT);
    if (values == NULL) {
        return 1;
    }
    crng_game_rng_seed(&rng, UINT64_C(1));
    for (index = 0; index < SAMPLE_COUNT; ++index) {
        values[index] = crng_game_rng_next_u64(&rng);
        if (index == 0) {
            if (crng_game_rng_last_roundoff(&rng, &roundoff) != CRNG_OK) {
                result = 1;
            }
        }
    }

    crng_test_analyze_distribution(&report, values, SAMPLE_COUNT);
    if (!crng_test_distribution_has_coverage(&report) ||
        roundoff.pi.remainder != UINT32_C(3488475904) ||
        roundoff.e.remainder != UINT32_C(222945867)) {
        result = 1;
    }
    crng_test_print_json("game_rng", &report);
    free(values);
    return result;
}

int main(void) {
    return run_game_rng_tests();
}
