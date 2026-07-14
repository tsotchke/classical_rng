#include "classical_rng/game_rng.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void crng_game_usage(const char *program) {
    fprintf(stderr,
            "usage: %s [--seed N] [--count N] [--show-roundoff]\n",
            program);
}

static int crng_parse_u64(const char *text, uint64_t *out) {
    char *end = NULL;
    unsigned long long value;

    if (text[0] == '-') {
        return 0;
    }
    errno = 0;
    value = strtoull(text, &end, 0);
    if (errno != 0 || end == text || *end != '\0') {
        return 0;
    }
    *out = (uint64_t)value;
    return 1;
}

int main(int argc, char **argv) {
    uint64_t seed = UINT64_C(2026);
    uint64_t count = UINT64_C(10);
    int show_roundoff = 0;
    crng_game_rng rng;
    int argument;

    for (argument = 1; argument < argc; ++argument) {
        if (strcmp(argv[argument], "--seed") == 0 && argument + 1 < argc) {
            if (!crng_parse_u64(argv[++argument], &seed)) {
                crng_game_usage(argv[0]);
                return 2;
            }
        } else if (strcmp(argv[argument], "--count") == 0 && argument + 1 < argc) {
            if (!crng_parse_u64(argv[++argument], &count)) {
                crng_game_usage(argv[0]);
                return 2;
            }
        } else if (strcmp(argv[argument], "--show-roundoff") == 0) {
            show_roundoff = 1;
        } else if (strcmp(argv[argument], "--help") == 0) {
            crng_game_usage(argv[0]);
            return 0;
        } else {
            crng_game_usage(argv[0]);
            return 2;
        }
    }

    crng_game_rng_seed(&rng, seed);
    while (count-- != 0) {
        const uint64_t value = crng_game_rng_next_u64(&rng);
        printf("%" PRIu64, value);
        if (show_roundoff != 0) {
            crng_roundoff_sample sample;
            (void)crng_game_rng_last_roundoff(&rng, &sample);
            printf(" pi=%" PRIu32 "/%" PRIu32 " e=%" PRIu32 "/%" PRIu32,
                   sample.pi.remainder,
                   sample.pi.divisor,
                   sample.e.remainder,
                   sample.e.divisor);
        }
        putchar('\n');
    }
    return 0;
}
