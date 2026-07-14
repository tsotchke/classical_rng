#include "classical_rng.h"

#include <stdio.h>

int main(void) {
    unsigned char token[32];
    size_t index;
    crng_status status = crng_secure_bytes(token, sizeof(token));

    if (status != CRNG_OK) {
        fprintf(stderr, "secure random source failed: %s\n", crng_status_string(status));
        return 1;
    }

    for (index = 0; index < sizeof(token); ++index) {
        printf("%02x", (unsigned int)token[index]);
    }
    putchar('\n');
    return 0;
}
