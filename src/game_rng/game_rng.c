#include "classical_rng.h"
#include "../common/constants.h"

#include <ctype.h>
#include <string.h>

#define CRNG_DECIMAL_LIMB_BASE UINT64_C(1000000000)

static const char crng_pi_decimal[] =
    "3.1415926535897932384626433832795028841971693993751058209749445923078164062862"
    "089986280348253421170679821480865132823066470938446095505822317253594081284811"
    "1745028410270193852110555964462294895493038196";

static const char crng_e_decimal[] =
    "2.7182818284590452353602874713526624977572470936999595749669676277240766303535"
    "475945713821785251664274274663919320030599218174135966290435729003342952605956"
    "3073813232862794349076323382988075319525101901";

static int crng_fixed_decimal_parse(
    crng_high_precision_decimal *out,
    const char *text
) {
    int after_decimal = 0;

    memset(out, 0, sizeof(*out));
    out->used = 1;
    while (*text != '\0') {
        size_t index;
        uint64_t carry;

        if (*text == '.') {
            if (after_decimal != 0) {
                return 0;
            }
            after_decimal = 1;
            ++text;
            continue;
        }
        if (!isdigit((unsigned char)*text)) {
            return 0;
        }

        carry = (uint64_t)(unsigned int)(*text - '0');
        for (index = 0; index < out->used; ++index) {
            const uint64_t value = (uint64_t)out->limbs[index] * UINT64_C(10) + carry;
            out->limbs[index] = (uint32_t)(value % CRNG_DECIMAL_LIMB_BASE);
            carry = value / CRNG_DECIMAL_LIMB_BASE;
        }
        if (carry != 0) {
            if (out->used == CRNG_HIGH_PRECISION_LIMBS) {
                return 0;
            }
            out->limbs[out->used] = (uint32_t)carry;
            ++out->used;
        }
        if (after_decimal != 0) {
            ++out->fractional_digits;
        }
        ++text;
    }
    return 1;
}

static uint32_t crng_fixed_decimal_div_small(
    const crng_high_precision_decimal *value,
    uint32_t divisor,
    crng_high_precision_decimal *quotient
) {
    uint64_t remainder = 0;
    size_t index;

    memset(quotient, 0, sizeof(*quotient));
    quotient->used = value->used;
    quotient->fractional_digits = value->fractional_digits;
    for (index = value->used; index > 0; --index) {
        const uint64_t current = remainder * CRNG_DECIMAL_LIMB_BASE
                                 + value->limbs[index - 1];
        quotient->limbs[index - 1] = (uint32_t)(current / divisor);
        remainder = current % divisor;
    }
    while (quotient->used > 1 && quotient->limbs[quotient->used - 1] == 0) {
        --quotient->used;
    }
    return (uint32_t)remainder;
}

static uint64_t crng_splitmix64(uint64_t *state) {
    uint64_t value;

    *state += CRNG_CONSTANT_PHI;
    value = *state;
    value = (value ^ (value >> 30)) * CRNG_CONSTANT_E;
    value = (value ^ (value >> 27)) * CRNG_CONSTANT_PI;
    return value ^ (value >> 31);
}

static uint64_t crng_rotate_left_64(uint64_t value, unsigned int shift) {
    return (value << shift) | (value >> (64U - shift));
}

static uint64_t crng_avalanche64(uint64_t value) {
    value = (value ^ (value >> 30)) * CRNG_CONSTANT_E;
    value = (value ^ (value >> 27)) * CRNG_CONSTANT_PI;
    return value ^ (value >> 31);
}

static uint32_t crng_roundoff_divisor(uint64_t value) {
    uint32_t divisor = (uint32_t)(value ^ (value >> 32));
    divisor |= UINT32_C(1);
    return divisor < UINT32_C(3) ? UINT32_C(3) : divisor;
}

static uint64_t crng_collect_roundoff(crng_game_rng *rng, uint64_t raw) {
    crng_high_precision_decimal quotient;
    const uint32_t pi_divisor = crng_roundoff_divisor(
        raw ^ rng->state[0] ^ rng->step
    );
    const uint32_t e_divisor = crng_roundoff_divisor(
        crng_rotate_left_64(raw, 29U) ^ rng->state[3] ^ (rng->step * CRNG_CONSTANT_E)
    );
    uint64_t residue_word;

    rng->last_roundoff.step = rng->step;
    rng->last_roundoff.pi.divisor = pi_divisor;
    rng->last_roundoff.pi.remainder = crng_fixed_decimal_div_small(
        &rng->pi,
        pi_divisor,
        &quotient
    );
    rng->last_roundoff.e.divisor = e_divisor;
    rng->last_roundoff.e.remainder = crng_fixed_decimal_div_small(
        &rng->e,
        e_divisor,
        &quotient
    );

    residue_word = ((uint64_t)rng->last_roundoff.pi.remainder << 32)
                   | rng->last_roundoff.e.remainder;
    residue_word ^= ((uint64_t)pi_divisor << 17);
    residue_word ^= crng_rotate_left_64((uint64_t)e_divisor, 43U);
    rng->last_roundoff.mixed_residue = crng_avalanche64(residue_word);
    return rng->last_roundoff.mixed_residue;
}

const char *crng_game_rng_constant_decimal(const char *name) {
    if (name == NULL) {
        return NULL;
    }
    if (strcmp(name, "pi") == 0) {
        return crng_pi_decimal;
    }
    if (strcmp(name, "e") == 0) {
        return crng_e_decimal;
    }
    return NULL;
}

crng_status crng_game_rng_last_roundoff(
    const crng_game_rng *rng,
    crng_roundoff_sample *out
) {
    if (rng == NULL || out == NULL) {
        return CRNG_ERR_NULL;
    }
    *out = rng->last_roundoff;
    return CRNG_OK;
}

void crng_game_rng_seed(crng_game_rng *rng, uint64_t seed) {
    size_t index;

    if (rng == NULL) {
        return;
    }

    for (index = 0; index < 4; ++index) {
        rng->state[index] = crng_splitmix64(&seed);
    }
    rng->step = 0;
    memset(&rng->last_roundoff, 0, sizeof(rng->last_roundoff));
    (void)crng_fixed_decimal_parse(&rng->pi, crng_pi_decimal);
    (void)crng_fixed_decimal_parse(&rng->e, crng_e_decimal);
}

crng_status crng_game_rng_seed_auto(crng_game_rng *rng) {
    crng_status status;
    uint64_t seed_material;

    if (rng == NULL) {
        return CRNG_ERR_NULL;
    }

    status = crng_secure_u64(&seed_material);
    if (status != CRNG_OK) {
        memset(rng, 0, sizeof(*rng));
        return status;
    }

    crng_game_rng_seed(rng, seed_material);
    seed_material = 0;
    return CRNG_OK;
}

uint64_t crng_game_rng_next_u64(crng_game_rng *rng) {
    const uint64_t raw = crng_rotate_left_64(rng->state[1] * UINT64_C(5), 7U)
                         * UINT64_C(9);
    const uint64_t temporary = rng->state[1] << 17;
    uint64_t roundoff;

    rng->state[2] ^= rng->state[0];
    rng->state[3] ^= rng->state[1];
    rng->state[1] ^= rng->state[2];
    rng->state[0] ^= rng->state[3];
    rng->state[2] ^= temporary;
    rng->state[3] = crng_rotate_left_64(rng->state[3], 45U);

    ++rng->step;
    roundoff = crng_collect_roundoff(rng, raw);
    return crng_avalanche64(raw ^ roundoff ^ (rng->step * CRNG_CONSTANT_ROOT2));
}

uint32_t crng_game_rng_next_u32(crng_game_rng *rng) {
    return (uint32_t)(crng_game_rng_next_u64(rng) >> 32);
}

crng_status crng_game_rng_fill(crng_game_rng *rng, void *buffer, size_t size) {
    unsigned char *bytes = (unsigned char *)buffer;

    if (rng == NULL || (buffer == NULL && size != 0)) {
        return CRNG_ERR_NULL;
    }

    while (size != 0) {
        uint64_t value = crng_game_rng_next_u64(rng);
        size_t index;
        size_t chunk = size < sizeof(value) ? size : sizeof(value);

        for (index = 0; index < chunk; ++index) {
            bytes[index] = (unsigned char)(value >> (index * 8U));
        }
        bytes += chunk;
        size -= chunk;
    }
    return CRNG_OK;
}

crng_status crng_game_rng_uniform_u64(
    crng_game_rng *rng,
    uint64_t upper_bound,
    uint64_t *out
) {
    uint64_t value;
    uint64_t threshold;

    if (rng == NULL || out == NULL) {
        return CRNG_ERR_NULL;
    }
    if (upper_bound == 0) {
        return CRNG_ERR_INVALID_RANGE;
    }

    threshold = (UINT64_C(0) - upper_bound) % upper_bound;
    do {
        value = crng_game_rng_next_u64(rng);
    } while (value < threshold);

    *out = value % upper_bound;
    return CRNG_OK;
}

crng_status crng_game_rng_range_i32(
    crng_game_rng *rng,
    int32_t minimum,
    int32_t maximum,
    int32_t *out
) {
    uint64_t offset;
    uint64_t span;
    crng_status status;

    if (rng == NULL || out == NULL) {
        return CRNG_ERR_NULL;
    }
    if (minimum > maximum) {
        return CRNG_ERR_INVALID_RANGE;
    }

    span = (uint64_t)((int64_t)maximum - (int64_t)minimum) + UINT64_C(1);
    status = crng_game_rng_uniform_u64(rng, span, &offset);
    if (status != CRNG_OK) {
        return status;
    }

    *out = (int32_t)((int64_t)minimum + (int64_t)offset);
    return CRNG_OK;
}

double crng_game_rng_next_double(crng_game_rng *rng) {
    return (double)(crng_game_rng_next_u64(rng) >> 11) * 0x1.0p-53;
}
