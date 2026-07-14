#include "classical_rng.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;
static int checks = 0;

#define CHECK(condition)                                                        \
    do {                                                                        \
        ++checks;                                                               \
        if (!(condition)) {                                                     \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            ++failures;                                                         \
        }                                                                       \
    } while (0)

static void test_reproducible_sequence(void) {
    static const uint64_t expected[] = {
        UINT64_C(0xbdfd01ff6a8c2511),
        UINT64_C(0x6ab3566f9b72364d),
        UINT64_C(0xe5ba6e07554a49ab),
        UINT64_C(0x00b2ccd9528e7e96)
    };
    crng_game_rng first;
    crng_game_rng second;
    size_t index;

    crng_game_rng_seed(&first, UINT64_C(1));
    crng_game_rng_seed(&second, UINT64_C(1));
    for (index = 0; index < sizeof(expected) / sizeof(expected[0]); ++index) {
        const uint64_t a = crng_game_rng_next_u64(&first);
        const uint64_t b = crng_game_rng_next_u64(&second);
        CHECK(a == expected[index]);
        CHECK(a == b);
    }
}

static void test_high_precision_roundoff(void) {
    crng_game_rng rng;
    crng_roundoff_sample sample;
    const char *pi = crng_game_rng_constant_decimal("pi");
    const char *e = crng_game_rng_constant_decimal("e");

    CHECK(pi != NULL);
    CHECK(e != NULL);
    CHECK(strlen(pi) == 202U);
    CHECK(strlen(e) == 202U);
    CHECK(strncmp(pi, "3.14159265358979323846", 22U) == 0);
    CHECK(strncmp(e, "2.71828182845904523536", 22U) == 0);
    CHECK(crng_game_rng_constant_decimal(NULL) == NULL);
    CHECK(crng_game_rng_constant_decimal("sqrt2") == NULL);

    crng_game_rng_seed(&rng, UINT64_C(1));
    CHECK(rng.pi.fractional_digits == 200U);
    CHECK(rng.e.fractional_digits == 200U);
    CHECK(rng.pi.used > 20U);
    CHECK(rng.e.used > 20U);
    CHECK(crng_game_rng_next_u64(&rng) == UINT64_C(0xbdfd01ff6a8c2511));
    CHECK(crng_game_rng_last_roundoff(&rng, &sample) == CRNG_OK);
    CHECK(sample.step == UINT64_C(1));
    CHECK(sample.pi.divisor == UINT32_C(3772506329));
    CHECK(sample.pi.remainder == UINT32_C(3488475904));
    CHECK(sample.e.divisor == UINT32_C(1911668157));
    CHECK(sample.e.remainder == UINT32_C(222945867));
    CHECK(sample.pi.remainder < sample.pi.divisor);
    CHECK(sample.e.remainder < sample.e.divisor);
    CHECK(sample.mixed_residue == UINT64_C(0x7dde1b23385df738));
    CHECK(crng_game_rng_last_roundoff(NULL, &sample) == CRNG_ERR_NULL);
    CHECK(crng_game_rng_last_roundoff(&rng, NULL) == CRNG_ERR_NULL);
}

static void test_seed_and_fill(void) {
    crng_game_rng first;
    crng_game_rng second;
    unsigned char a[19];
    unsigned char b[19];

    crng_game_rng_seed(NULL, UINT64_C(7));
    crng_game_rng_seed(&first, UINT64_C(7));
    crng_game_rng_seed(&second, UINT64_C(7));
    CHECK(crng_game_rng_fill(&first, a, sizeof(a)) == CRNG_OK);
    CHECK(crng_game_rng_fill(&second, b, sizeof(b)) == CRNG_OK);
    CHECK(memcmp(a, b, sizeof(a)) == 0);
    CHECK(crng_game_rng_fill(&first, NULL, 0) == CRNG_OK);
    CHECK(crng_game_rng_fill(&first, NULL, 1) == CRNG_ERR_NULL);
    CHECK(crng_game_rng_fill(NULL, a, sizeof(a)) == CRNG_ERR_NULL);
}

static void test_unbiased_bounds(void) {
    enum { BUCKETS = 7, SAMPLES = 210000 };
    crng_game_rng rng;
    uint64_t counts[BUCKETS] = {0};
    uint64_t value = UINT64_MAX;
    int index;

    crng_game_rng_seed(&rng, UINT64_C(0x123456789abcdef0));
    CHECK(crng_game_rng_uniform_u64(&rng, 0, &value) == CRNG_ERR_INVALID_RANGE);
    CHECK(crng_game_rng_uniform_u64(&rng, 10, NULL) == CRNG_ERR_NULL);
    CHECK(crng_game_rng_uniform_u64(NULL, 10, &value) == CRNG_ERR_NULL);
    CHECK(crng_game_rng_uniform_u64(&rng, 1, &value) == CRNG_OK);
    CHECK(value == 0);

    for (index = 0; index < SAMPLES; ++index) {
        CHECK(crng_game_rng_uniform_u64(&rng, BUCKETS, &value) == CRNG_OK);
        ++counts[value];
    }

    for (index = 0; index < BUCKETS; ++index) {
        const uint64_t expected = SAMPLES / BUCKETS;
        const uint64_t difference = counts[index] > expected
            ? counts[index] - expected
            : expected - counts[index];
        CHECK(difference < expected / 25U); /* deterministic four-percent smoke */
    }
}

static void test_signed_ranges(void) {
    crng_game_rng rng;
    int32_t value = 99;
    int saw_negative = 0;
    int saw_nonnegative = 0;
    int index;

    crng_game_rng_seed(&rng, UINT64_C(42));
    CHECK(crng_game_rng_range_i32(&rng, 5, 5, &value) == CRNG_OK);
    CHECK(value == 5);
    CHECK(crng_game_rng_range_i32(&rng, 7, 3, &value) == CRNG_ERR_INVALID_RANGE);
    CHECK(value == 5);
    CHECK(crng_game_rng_range_i32(&rng, 0, -1, &value) == CRNG_ERR_INVALID_RANGE);
    CHECK(crng_game_rng_range_i32(&rng, -3, 6, NULL) == CRNG_ERR_NULL);
    CHECK(crng_game_rng_range_i32(NULL, -3, 6, &value) == CRNG_ERR_NULL);

    /* Regression for GitHub PR #1: this span is exactly 2^32. */
    for (index = 0; index < 100000; ++index) {
        CHECK(crng_game_rng_range_i32(&rng, INT32_MIN, INT32_MAX, &value) == CRNG_OK);
        saw_negative |= value < 0;
        saw_nonnegative |= value >= 0;
    }
    CHECK(saw_negative);
    CHECK(saw_nonnegative);
}

static void test_floating_point(void) {
    crng_game_rng rng;
    int index;

    crng_game_rng_seed(&rng, UINT64_C(1234));
    for (index = 0; index < 10000; ++index) {
        const double value = crng_game_rng_next_double(&rng);
        CHECK(value >= 0.0);
        CHECK(value < 1.0);
    }
}

static void test_operating_system_randomness(void) {
    crng_game_rng automatic;
    unsigned char first[32] = {0};
    unsigned char second[32] = {0};
    uint64_t bounded = UINT64_MAX;
    int32_t signed_value = 0;

    CHECK(crng_secure_bytes(NULL, 0) == CRNG_OK);
    CHECK(crng_secure_bytes(NULL, 1) == CRNG_ERR_NULL);
    CHECK(crng_secure_bytes(first, sizeof(first)) == CRNG_OK);
    CHECK(crng_secure_bytes(second, sizeof(second)) == CRNG_OK);
    CHECK(memcmp(first, second, sizeof(first)) != 0);
    CHECK(crng_secure_u64(NULL) == CRNG_ERR_NULL);
    CHECK(crng_secure_uniform_u64(0, &bounded) == CRNG_ERR_INVALID_RANGE);
    CHECK(crng_secure_uniform_u64(1, &bounded) == CRNG_OK);
    CHECK(bounded == 0);
    CHECK(crng_secure_range_i32(9, 4, &signed_value) == CRNG_ERR_INVALID_RANGE);
    CHECK(crng_secure_range_i32(INT32_MIN, INT32_MAX, &signed_value) == CRNG_OK);
    CHECK(crng_game_rng_seed_auto(&automatic) == CRNG_OK);
    CHECK((automatic.state[0] | automatic.state[1] |
           automatic.state[2] | automatic.state[3]) != 0);
    CHECK(crng_game_rng_seed_auto(NULL) == CRNG_ERR_NULL);
}

static void test_number_theory_layer(void) {
    uint64_t prime = 0;

    CHECK(!crng_crypto_is_prime_u64(UINT64_C(0)));
    CHECK(!crng_crypto_is_prime_u64(UINT64_C(1)));
    CHECK(crng_crypto_is_prime_u64(UINT64_C(2)));
    CHECK(crng_crypto_is_prime_u64(UINT64_C(37)));
    CHECK(!crng_crypto_is_prime_u64(UINT64_C(561))); /* Carmichael number */
    CHECK(!crng_crypto_is_prime_u64(UINT64_MAX));
    CHECK(crng_crypto_is_prime_u64(UINT64_C(18446744073709551557)));
    CHECK(crng_crypto_random_prime_u64(17, 17, 1, &prime) == CRNG_OK);
    CHECK(prime == UINT64_C(17));
    CHECK(crng_crypto_random_prime_u64(14, 16, 8, &prime) == CRNG_ERR_NOT_FOUND);
    CHECK(crng_crypto_random_prime_u64(9, 4, 8, &prime) == CRNG_ERR_INVALID_RANGE);
    CHECK(crng_crypto_random_prime_u64(2, 10, 0, &prime) == CRNG_ERR_INVALID_RANGE);
    CHECK(crng_crypto_random_prime_u64(2, 10, 8, NULL) == CRNG_ERR_NULL);
}

static void test_status_strings(void) {
    CHECK(strcmp(crng_status_string(CRNG_OK), "success") == 0);
    CHECK(strcmp(crng_status_string(CRNG_ERR_INVALID_RANGE), "invalid range") == 0);
    CHECK(strcmp(crng_status_string(CRNG_ERR_NOT_FOUND),
                 "no matching value found within the attempt limit") == 0);
    CHECK(strcmp(crng_status_string((crng_status)99),
                 "unknown classical_rng status") == 0);
}

int main(void) {
    test_reproducible_sequence();
    test_high_precision_roundoff();
    test_seed_and_fill();
    test_unbiased_bounds();
    test_signed_ranges();
    test_floating_point();
    test_operating_system_randomness();
    test_number_theory_layer();
    test_status_strings();

    if (failures != 0) {
        fprintf(stderr, "%d of %d checks failed\n", failures, checks);
        return 1;
    }

    printf("classical_rng: %d checks passed\n", checks);
    return 0;
}
