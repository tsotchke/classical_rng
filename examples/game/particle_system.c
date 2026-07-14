#include "classical_rng/game_rng.h"

#include <stdio.h>

typedef struct particle {
    double x;
    double y;
    double velocity_x;
    double velocity_y;
} particle;

int main(void) {
    particle particles[8];
    crng_game_rng rng;
    size_t index;

    crng_game_rng_seed(&rng, UINT64_C(0x5041525449434c45));
    for (index = 0; index < sizeof(particles) / sizeof(particles[0]); ++index) {
        particles[index].x = 0.0;
        particles[index].y = 0.0;
        particles[index].velocity_x = crng_game_rng_next_double(&rng) * 2.0 - 1.0;
        particles[index].velocity_y = crng_game_rng_next_double(&rng) * 2.0 - 1.0;
    }

    for (index = 0; index < sizeof(particles) / sizeof(particles[0]); ++index) {
        particles[index].x += particles[index].velocity_x;
        particles[index].y += particles[index].velocity_y;
        printf("particle %zu: position=(%.6f, %.6f)\n",
               index,
               particles[index].x,
               particles[index].y);
    }
    return 0;
}
