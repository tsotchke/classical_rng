#include "classical_rng/crypto_rng.h"

#include <errno.h>
#include <limits.h>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <bcrypt.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || \
      defined(__NetBSD__) || defined(__DragonFly__)
#  include <stdlib.h>
#else
#  include <fcntl.h>
#  include <unistd.h>
#  if defined(__linux__) && !defined(__ANDROID__)
#    include <sys/random.h>
#    define CRNG_USE_GETRANDOM 1
#  elif defined(__ANDROID__) && defined(__ANDROID_API__) && \
        __ANDROID_API__ >= 28
#    include <sys/random.h>
#    define CRNG_USE_GETRANDOM 1
#  endif
#endif

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__FreeBSD__) && \
    !defined(__OpenBSD__) && !defined(__NetBSD__) && !defined(__DragonFly__)
static crng_status crng_read_urandom(unsigned char *buffer, size_t size) {
    int descriptor;
    int flags = O_RDONLY;

#  if defined(O_CLOEXEC)
    flags |= O_CLOEXEC;
#  endif
    descriptor = open("/dev/urandom", flags);
    if (descriptor < 0) {
        return CRNG_ERR_SYSTEM;
    }

    while (size != 0) {
        const size_t chunk = size > (size_t)1048576 ? (size_t)1048576 : size;
        ssize_t count = read(descriptor, buffer, chunk);
        if (count > 0) {
            buffer += (size_t)count;
            size -= (size_t)count;
        } else if (count < 0 && errno == EINTR) {
            continue;
        } else {
            (void)close(descriptor);
            return CRNG_ERR_SYSTEM;
        }
    }

    (void)close(descriptor);
    return CRNG_OK;
}
#endif

crng_status crng_secure_bytes(void *buffer, size_t size) {
    unsigned char *bytes = (unsigned char *)buffer;

    if (size == 0) {
        return CRNG_OK;
    }
    if (buffer == NULL) {
        return CRNG_ERR_NULL;
    }

#if defined(_WIN32)
    while (size != 0) {
        const ULONG chunk = size > (size_t)ULONG_MAX ? ULONG_MAX : (ULONG)size;
        const NTSTATUS result = BCryptGenRandom(
            NULL,
            bytes,
            chunk,
            BCRYPT_USE_SYSTEM_PREFERRED_RNG
        );
        if (result < 0) {
            return CRNG_ERR_SYSTEM;
        }
        bytes += chunk;
        size -= chunk;
    }
    return CRNG_OK;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || \
      defined(__NetBSD__) || defined(__DragonFly__)
    arc4random_buf(bytes, size);
    return CRNG_OK;
#elif defined(CRNG_USE_GETRANDOM)
    while (size != 0) {
        ssize_t count = getrandom(bytes, size, 0);
        if (count > 0) {
            bytes += (size_t)count;
            size -= (size_t)count;
        } else if (count < 0 && errno == EINTR) {
            continue;
        } else if (count < 0 && errno == ENOSYS) {
            return crng_read_urandom(bytes, size);
        } else {
            return CRNG_ERR_SYSTEM;
        }
    }
    return CRNG_OK;
#else
    return crng_read_urandom(bytes, size);
#endif
}

crng_status crng_secure_u64(uint64_t *out) {
    if (out == NULL) {
        return CRNG_ERR_NULL;
    }
    return crng_secure_bytes(out, sizeof(*out));
}

crng_status crng_secure_uniform_u64(uint64_t upper_bound, uint64_t *out) {
    uint64_t value;
    uint64_t threshold;
    crng_status status;

    if (out == NULL) {
        return CRNG_ERR_NULL;
    }
    if (upper_bound == 0) {
        return CRNG_ERR_INVALID_RANGE;
    }

    threshold = (UINT64_C(0) - upper_bound) % upper_bound;
    do {
        status = crng_secure_u64(&value);
        if (status != CRNG_OK) {
            return status;
        }
    } while (value < threshold);

    *out = value % upper_bound;
    return CRNG_OK;
}

crng_status crng_secure_range_i32(
    int32_t minimum,
    int32_t maximum,
    int32_t *out
) {
    uint64_t offset;
    uint64_t span;
    crng_status status;

    if (out == NULL) {
        return CRNG_ERR_NULL;
    }
    if (minimum > maximum) {
        return CRNG_ERR_INVALID_RANGE;
    }

    span = (uint64_t)((int64_t)maximum - (int64_t)minimum) + UINT64_C(1);
    status = crng_secure_uniform_u64(span, &offset);
    if (status != CRNG_OK) {
        return status;
    }

    *out = (int32_t)((int64_t)minimum + (int64_t)offset);
    return CRNG_OK;
}

static uint64_t crng_add_mod_u64(uint64_t left, uint64_t right, uint64_t modulus) {
    return left >= modulus - right ? left - (modulus - right) : left + right;
}

static uint64_t crng_mul_mod_u64(uint64_t left, uint64_t right, uint64_t modulus) {
    uint64_t result = 0;

    left %= modulus;
    while (right != 0) {
        if ((right & UINT64_C(1)) != 0) {
            result = crng_add_mod_u64(result, left, modulus);
        }
        right >>= 1;
        if (right != 0) {
            left = crng_add_mod_u64(left, left, modulus);
        }
    }
    return result;
}

static uint64_t crng_pow_mod_u64(uint64_t base, uint64_t exponent, uint64_t modulus) {
    uint64_t result = UINT64_C(1);

    base %= modulus;
    while (exponent != 0) {
        if ((exponent & UINT64_C(1)) != 0) {
            result = crng_mul_mod_u64(result, base, modulus);
        }
        exponent >>= 1;
        if (exponent != 0) {
            base = crng_mul_mod_u64(base, base, modulus);
        }
    }
    return result;
}

bool crng_crypto_is_prime_u64(uint64_t value) {
    static const uint64_t small_primes[] = {
        UINT64_C(2), UINT64_C(3), UINT64_C(5), UINT64_C(7),
        UINT64_C(11), UINT64_C(13), UINT64_C(17), UINT64_C(19),
        UINT64_C(23), UINT64_C(29), UINT64_C(31), UINT64_C(37)
    };
    static const uint64_t witnesses[] = {
        UINT64_C(2), UINT64_C(325), UINT64_C(9375), UINT64_C(28178),
        UINT64_C(450775), UINT64_C(9780504), UINT64_C(1795265022)
    };
    uint64_t odd_part;
    unsigned int powers_of_two = 0;
    size_t index;

    if (value < UINT64_C(2)) {
        return false;
    }
    for (index = 0; index < sizeof(small_primes) / sizeof(small_primes[0]); ++index) {
        if (value == small_primes[index]) {
            return true;
        }
        if (value % small_primes[index] == 0) {
            return false;
        }
    }

    odd_part = value - UINT64_C(1);
    while ((odd_part & UINT64_C(1)) == 0) {
        odd_part >>= 1;
        ++powers_of_two;
    }

    for (index = 0; index < sizeof(witnesses) / sizeof(witnesses[0]); ++index) {
        const uint64_t witness = witnesses[index] % value;
        uint64_t result;
        unsigned int round;

        if (witness == 0) {
            continue;
        }
        result = crng_pow_mod_u64(witness, odd_part, value);
        if (result == UINT64_C(1) || result == value - UINT64_C(1)) {
            continue;
        }

        for (round = 1; round < powers_of_two; ++round) {
            result = crng_mul_mod_u64(result, result, value);
            if (result == value - UINT64_C(1)) {
                break;
            }
        }
        if (round == powers_of_two) {
            return false;
        }
    }
    return true;
}

crng_status crng_crypto_random_prime_u64(
    uint64_t minimum,
    uint64_t maximum,
    uint64_t max_attempts,
    uint64_t *out
) {
    uint64_t attempt;

    if (out == NULL) {
        return CRNG_ERR_NULL;
    }
    if (minimum > maximum || max_attempts == 0 || maximum < UINT64_C(2)) {
        return CRNG_ERR_INVALID_RANGE;
    }
    if (minimum < UINT64_C(2)) {
        minimum = UINT64_C(2);
    }

    for (attempt = 0; attempt < max_attempts; ++attempt) {
        uint64_t candidate;
        crng_status status;

        const uint64_t span = maximum - minimum + UINT64_C(1);
        uint64_t offset;
        status = crng_secure_uniform_u64(span, &offset);
        if (status != CRNG_OK) {
            return status;
        }
        candidate = minimum + offset;
        if (crng_crypto_is_prime_u64(candidate)) {
            *out = candidate;
            return CRNG_OK;
        }
    }
    return CRNG_ERR_NOT_FOUND;
}
