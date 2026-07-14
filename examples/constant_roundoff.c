#include "classical_rng.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    crng_game_rng rng;
    crng_roundoff_sample sample;
    unsigned int index;

    crng_game_rng_seed(&rng, UINT64_C(1));
    printf("pi precision: %zu fractional digits\n",
           strlen(crng_game_rng_constant_decimal("pi")) - 2U);
    printf("e precision:  %zu fractional digits\n",
           strlen(crng_game_rng_constant_decimal("e")) - 2U);
    for (index = 0; index < 4U; ++index) {
        const uint64_t value = crng_game_rng_next_u64(&rng);
        if (crng_game_rng_last_roundoff(&rng, &sample) != CRNG_OK) {
            return 1;
        }
        printf(
            "step %" PRIu64 ": pi=%" PRIu32 "/%" PRIu32
            " e=%" PRIu32 "/%" PRIu32 " residue=%016" PRIx64
            " output=%016" PRIx64 "\n",
            sample.step,
            sample.pi.remainder,
            sample.pi.divisor,
            sample.e.remainder,
            sample.e.divisor,
            sample.mixed_residue,
            value
        );
    }
    return 0;
}
