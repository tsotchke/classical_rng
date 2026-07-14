#include <classical_rng.h>

int main(void) {
    crng_game_rng rng;
    crng_game_rng_seed(&rng, UINT64_C(5));
    return crng_game_rng_next_u64(&rng) == 0 ? 1 : 0;
}
