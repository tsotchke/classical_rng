#include "crypto_rng.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [OPTIONS]\n\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --num_values N        Number of random values to generate (default: 10)\n");
    fprintf(stderr, "  --prime_lower N       Lower bound for prime numbers (default: 2^64)\n");
    fprintf(stderr, "  --prime_upper N       Upper bound for prime numbers (default: 2^65)\n");
    fprintf(stderr, "  --mixing_rounds N     Number of mixing rounds (default: %d)\n", DEFAULT_MIXING_ROUNDS);
    fprintf(stderr, "  --format FORMAT       Output format: decimal, hex (default: decimal)\n");
    fprintf(stderr, "  --benchmark           Run performance benchmark\n");
    fprintf(stderr, "  --benchmark_size N    Number of iterations for benchmark (default: 10000000)\n");
    fprintf(stderr, "  --quiet               Suppress additional output\n");
    fprintf(stderr, "  --help                Display this help message\n");
}

int main(int argc, char *argv[]) {
    int num_values = 10;
    uint64_t prime_lower = 1ULL << (MIN_PRIME_BITS - 1);  // Fixed shift count
    uint64_t prime_upper = 1ULL << MIN_PRIME_BITS;        // Fixed shift count
    int mixing_rounds = DEFAULT_MIXING_ROUNDS;
    bool hex_output = false;
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
        } else if (strcmp(argv[i], "--prime_lower") == 0 && i + 1 < argc) {
            prime_lower = strtoull(argv[i + 1], NULL, 0);
            i++;
        } else if (strcmp(argv[i], "--prime_upper") == 0 && i + 1 < argc) {
            prime_upper = strtoull(argv[i + 1], NULL, 0);
            i++;
        } else if (strcmp(argv[i], "--mixing_rounds") == 0 && i + 1 < argc) {
            mixing_rounds = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--format") == 0 && i + 1 < argc) {
            if (strcmp(argv[i + 1], "hex") == 0) {
                hex_output = true;
            }
            i++;
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
    
    if (num_values <= 0 || prime_lower < 2 || prime_upper <= prime_lower || 
        mixing_rounds < 1 || benchmark_size <= 0) {
        fprintf(stderr, "Invalid parameters\n");
        print_usage(argv[0]);
        return 1;
    }
    
    if (do_benchmark) {
        if (!quiet) {
            fprintf(stderr, "Running benchmark with %d iterations...\n", benchmark_size);
        }
        clock_t start = clock();
        volatile uint64_t dummy = 0;
        for (int i = 0; i < benchmark_size; i++) {
            dummy ^= secure_random(prime_lower, prime_upper, mixing_rounds);
        }
        clock_t end = clock();
        double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Generated %d random numbers in %.3f seconds\n", benchmark_size, time_spent);
        printf("Speed: %.2f million numbers per second\n", benchmark_size / (time_spent * 1000000));
        return 0;
    }
    
    if (!quiet) {
        fprintf(stderr, "Generating %d random values...\n", num_values);
    }
    
    for (int i = 0; i < num_values; i++) {
        uint64_t value = secure_random(prime_lower, prime_upper, mixing_rounds);
        if (hex_output) {
            printf("0x%016llx\n", (unsigned long long)value);
        } else {
            printf("%llu\n", (unsigned long long)value);
        }
    }
    
    return 0;
}