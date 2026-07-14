#include "classical_rng.h"

#include <cstdint>

int main() {
    crng_game_rng rng = {};
    std::uint64_t value = 0;

    crng_game_rng_seed(&rng, UINT64_C(17));
    if (crng_game_rng_uniform_u64(&rng, UINT64_C(10), &value) != CRNG_OK) {
        return 1;
    }
    return value < UINT64_C(10) ? 0 : 2;
}
