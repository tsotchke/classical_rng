#include "game_rng.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [OPTIONS]\n\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --num_values N        Number of random values to generate (default: 10)\n");
    fprintf(stderr, "  --format FORMAT       Output format: decimal, hex (default: decimal)\n");
    fprintf(stderr, "  --range_min N         Minimum value for range output (default: 0)\n");
    fprintf(stderr, "  --range_max N         Maximum value for range output (default: no range)\n");
    fprintf(stderr, "  --float               Output as floating point numbers [0,1]\n");
    fprintf(stderr, "  --benchmark           Run performance benchmark\n");
    fprintf(stderr, "  --benchmark_size N    Number of iterations for benchmark (default: 10000000)\n");
    fprintf(stderr, "  --quiet               Suppress additional output\n");
    fprintf(stderr, "  --help                Display this help message\n");
}

int main(int argc, char *argv[]) {
    int num_values = 10;
    int range_min = 0;
    int range_max = -1;
    bool use_float = false;
    bool use_hex = false;
    bool quiet = false;
    bool do_benchmark = false;
    int benchmark_size = 10000000;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--num_values") == 0 && i + 1 < argc) {
            num_values = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--range_min") == 0 && i + 1 < argc) {
            range_min = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--range_max") == 0 && i + 1 < argc) {
            range_max = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--format") == 0 && i + 1 < argc) {
            if (strcmp(argv[i + 1], "hex") == 0) {
                use_hex = true;
            }
            i++;
        } else if (strcmp(argv[i], "--float") == 0) {
            use_float = true;
        } else if (strcmp(argv[i], "--quiet") == 0) {
            quiet = true;
        } else if (strcmp(argv[i], "--benchmark") == 0) {
            do_benchmark = true;
        } else if (strcmp(argv[i], "--benchmark_size") == 0 && i + 1 < argc) {
            benchmark_size = atoi(argv[i + 1]);
            i++;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    GameRNG rng;
    init_game_rng(&rng);
    
    if (do_benchmark) {
        run_benchmark(&rng, benchmark_size);
        return 0;
    }
    
    if (!quiet) {
        fprintf(stderr, "Generating %d random values...\n", num_values);
    }
    
    for (int i = 0; i < num_values; i++) {
        if (use_float) {
            printf("%f\n", random_float(&rng));
        } else if (range_max != -1) {
            printf("%d\n", random_range(&rng, range_min, range_max));
        } else {
            uint64_t value = next_random(&rng);
            if (use_hex) {
                printf("0x%016llx\n", (unsigned long long)value);
            } else {
                printf("%llu\n", (unsigned long long)value);
            }
        }
    }
    
    return 0;
}