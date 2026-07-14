#include "classical_rng/crypto_rng.h"

#include <stdio.h>

int main(void) {
    unsigned char token[32];
    size_t index;
    crng_status status = crng_secure_bytes(token, sizeof(token));

    if (status != CRNG_OK) {
        fprintf(stderr, "token generation failed: %s\n", crng_status_string(status));
        return 1;
    }
    printf("256-bit token: ");
    for (index = 0; index < sizeof(token); ++index) {
        printf("%02x", (unsigned int)token[index]);
    }
    putchar('\n');
    return 0;
}
