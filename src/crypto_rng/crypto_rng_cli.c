#include "classical_rng/crypto_rng.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void crng_crypto_usage(const char *program) {
    fprintf(stderr,
            "usage: %s [--bytes N | --prime MIN MAX] [--attempts N]\n",
            program);
}

static int crng_crypto_parse_u64(const char *text, uint64_t *out) {
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
    uint64_t byte_count = UINT64_C(32);
    uint64_t minimum = 0;
    uint64_t maximum = 0;
    uint64_t attempts = UINT64_C(1024);
    int prime_mode = 0;
    int argument;

    for (argument = 1; argument < argc; ++argument) {
        if (strcmp(argv[argument], "--bytes") == 0 && argument + 1 < argc) {
            if (!crng_crypto_parse_u64(argv[++argument], &byte_count)) {
                crng_crypto_usage(argv[0]);
                return 2;
            }
        } else if (strcmp(argv[argument], "--prime") == 0 && argument + 2 < argc) {
            prime_mode = 1;
            if (!crng_crypto_parse_u64(argv[++argument], &minimum) ||
                !crng_crypto_parse_u64(argv[++argument], &maximum)) {
                crng_crypto_usage(argv[0]);
                return 2;
            }
        } else if (strcmp(argv[argument], "--attempts") == 0 && argument + 1 < argc) {
            if (!crng_crypto_parse_u64(argv[++argument], &attempts)) {
                crng_crypto_usage(argv[0]);
                return 2;
            }
        } else if (strcmp(argv[argument], "--help") == 0) {
            crng_crypto_usage(argv[0]);
            return 0;
        } else {
            crng_crypto_usage(argv[0]);
            return 2;
        }
    }

    if (prime_mode != 0) {
        uint64_t prime;
        crng_status status = crng_crypto_random_prime_u64(
            minimum,
            maximum,
            attempts,
            &prime
        );
        if (status != CRNG_OK) {
            fprintf(stderr, "prime generation failed: %s\n", crng_status_string(status));
            return 1;
        }
        printf("%" PRIu64 "\n", prime);
        return 0;
    }

    if (byte_count > UINT64_C(1048576)) {
        fprintf(stderr, "refusing to print more than 1048576 bytes\n");
        return 2;
    }
    while (byte_count != 0) {
        unsigned char buffer[256];
        const size_t chunk = byte_count > sizeof(buffer)
            ? sizeof(buffer)
            : (size_t)byte_count;
        size_t index;
        crng_status status = crng_secure_bytes(buffer, chunk);
        if (status != CRNG_OK) {
            fprintf(stderr, "secure random source failed: %s\n", crng_status_string(status));
            return 1;
        }
        for (index = 0; index < chunk; ++index) {
            printf("%02x", (unsigned int)buffer[index]);
        }
        byte_count -= chunk;
    }
    putchar('\n');
    return 0;
}
