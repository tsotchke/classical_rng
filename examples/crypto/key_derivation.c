#include "classical_rng/crypto_rng.h"

#include <stdio.h>

/*
 * A KDF needs a reviewed cryptographic implementation (Argon2id, scrypt,
 * PBKDF2, or a protocol-specific HKDF). classical_rng supplies the random
 * salt input; it intentionally does not invent a KDF.
 */
int main(void) {
    unsigned char salt[16];
    size_t index;
    crng_status status = crng_secure_bytes(salt, sizeof(salt));

    if (status != CRNG_OK) {
        fprintf(stderr, "salt generation failed: %s\n", crng_status_string(status));
        return 1;
    }
    printf("128-bit KDF salt: ");
    for (index = 0; index < sizeof(salt); ++index) {
        printf("%02x", (unsigned int)salt[index]);
    }
    printf("\nPass this salt and the password to a reviewed KDF.\n");
    return 0;
}
