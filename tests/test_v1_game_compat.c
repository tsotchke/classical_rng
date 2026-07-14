#define CRNG_ENABLE_V1_COMPAT
#include "../src/game_rng/game_rng.h"

#include <limits.h>

int main(void) {
    GameRNG rng;
    int value;

    init_game_rng(&rng);
    (void)next_random(&rng);
    value = random_range(&rng, INT_MIN, INT_MAX);
    (void)value;
    if (random_range(&rng, 7, 3) != 7) {
        return 2;
    }
    return random_float(&rng) < 1.0 ? 0 : 3;
}
