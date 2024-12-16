#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../../src/game_rng/game_rng.h"

#define MAP_WIDTH  256
#define MAP_HEIGHT 256
#define MAX_HEIGHT 100.0f

// Simple heightmap-based terrain generator
int main() {
    float *heightmap = malloc(MAP_WIDTH * MAP_HEIGHT * sizeof(float));
    if (!heightmap) {
        fprintf(stderr, "Failed to allocate heightmap\n");
        return 1;
    }
    
    GameRNG rng;
    init_game_rng(&rng);
    
    // Generate base heightmap
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            // Use random numbers to generate terrain height
            float r1 = (float)next_random(&rng) / UINT64_MAX;
            float r2 = (float)next_random(&rng) / UINT64_MAX;
            
            // Simple noise function
            float height = r1 * (1.0f - r2 * 0.5f) * MAX_HEIGHT;
            
            // Distance from center for island-like formation
            float dx = (x - MAP_WIDTH/2) / (float)(MAP_WIDTH/2);
            float dy = (y - MAP_HEIGHT/2) / (float)(MAP_HEIGHT/2);
            float distance = sqrtf(dx*dx + dy*dy);
            
            // Attenuate height based on distance from center
            height *= (1.0f - distance * 0.8f);
            
            heightmap[y * MAP_WIDTH + x] = height > 0.0f ? height : 0.0f;
        }
    }
    
    // Output as simple grayscale PGM image
    printf("P2\n%d %d\n255\n", MAP_WIDTH, MAP_HEIGHT);
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            int height = (int)(heightmap[y * MAP_WIDTH + x] * 255.0f / MAX_HEIGHT);
            printf("%d ", height);
        }
        printf("\n");
    }
    
    free(heightmap);
    return 0;
}
