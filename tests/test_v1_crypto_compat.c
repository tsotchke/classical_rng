#define CRNG_ENABLE_V1_COMPAT
#include "../src/crypto_rng/crypto_rng.h"

int main(void) {
    uint64_t value = secure_random(UINT64_C(1000), UINT64_C(2000), 5);
    uint64_t prime = generate_random_prime(UINT64_C(17), UINT64_C(17), 1);

    (void)value;
    if (!is_prime(prime, 5) || prime != UINT64_C(17)) {
        return 1;
    }
    return 0;
}
