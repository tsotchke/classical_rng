#include "classical_rng.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    uint64_t seed = UINT64_C(2026);
    crng_game_rng rng;
    int roll;

    if (argc == 2) {
        seed = (uint64_t)strtoull(argv[1], NULL, 0);
    } else if (argc > 2) {
        fprintf(stderr, "usage: %s [seed]\n", argv[0]);
        return 2;
    }

    crng_game_rng_seed(&rng, seed);
    printf("seed: %" PRIu64 "\nrolls:", seed);
    for (roll = 0; roll < 12; ++roll) {
        uint64_t face;
        if (crng_game_rng_uniform_u64(&rng, 6, &face) != CRNG_OK) {
            return 1;
        }
        printf(" %" PRIu64, face + 1);
    }
    putchar('\n');
    return 0;
}
