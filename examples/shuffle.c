#include "classical_rng.h"

#include <inttypes.h>
#include <stdio.h>

int main(void) {
    int cards[10];
    crng_game_rng rng;
    size_t index;

    for (index = 0; index < sizeof(cards) / sizeof(cards[0]); ++index) {
        cards[index] = (int)index + 1;
    }

    /* A fixed seed makes this example and its bugs reproducible. */
    crng_game_rng_seed(&rng, UINT64_C(0xc0ffee));
    for (index = sizeof(cards) / sizeof(cards[0]); index > 1; --index) {
        uint64_t chosen;
        int temporary;
        crng_status status = crng_game_rng_uniform_u64(&rng, (uint64_t)index, &chosen);
        if (status != CRNG_OK) {
            fprintf(stderr, "shuffle failed: %s\n", crng_status_string(status));
            return 1;
        }
        temporary = cards[index - 1];
        cards[index - 1] = cards[(size_t)chosen];
        cards[(size_t)chosen] = temporary;
    }

    printf("shuffled values:");
    for (index = 0; index < sizeof(cards) / sizeof(cards[0]); ++index) {
        printf(" %d", cards[index]);
    }
    putchar('\n');
    return 0;
}
