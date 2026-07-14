#include "classical_rng/game_rng.h"

#include <stdio.h>

int main(void) {
    static const char levels[] = " .:-=+*#%@";
    crng_game_rng rng;
    int row;

    crng_game_rng_seed(&rng, UINT64_C(0x5445525241494e));
    for (row = 0; row < 8; ++row) {
        int column;
        for (column = 0; column < 32; ++column) {
            uint64_t height;
            if (crng_game_rng_uniform_u64(
                    &rng,
                    (uint64_t)(sizeof(levels) - 1U),
                    &height
                ) != CRNG_OK) {
                return 1;
            }
            putchar(levels[height]);
        }
        putchar('\n');
    }
    return 0;
}
