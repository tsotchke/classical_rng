#ifndef CLASSICAL_RNG_COMMON_H
#define CLASSICAL_RNG_COMMON_H

#include <float.h>
#include <limits.h>

#if CHAR_BIT != 8
#  error "classical_rng requires 8-bit bytes"
#endif

#if DBL_MANT_DIG < 53
#  error "classical_rng requires double with at least 53 significant bits"
#endif

#define CRNG_VERSION_MAJOR 2
#define CRNG_VERSION_MINOR 0
#define CRNG_VERSION_PATCH 0
#define CRNG_VERSION_STRING "2.0.0"

#if defined(_WIN32) && defined(CRNG_SHARED)
#  if defined(CRNG_BUILDING_LIBRARY)
#    define CRNG_API __declspec(dllexport)
#  else
#    define CRNG_API __declspec(dllimport)
#  endif
#elif defined(__GNUC__) && defined(CRNG_BUILDING_LIBRARY)
#  define CRNG_API __attribute__((visibility("default")))
#else
#  define CRNG_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Status returned by operations that can reject input or fail. */
typedef enum crng_status {
    /** The operation completed successfully. */
    CRNG_OK = 0,
    /** A required pointer was null. */
    CRNG_ERR_NULL = -1,
    /** A bound, interval, or attempt count was invalid. */
    CRNG_ERR_INVALID_RANGE = -2,
    /** The operating-system random source failed. */
    CRNG_ERR_SYSTEM = -3,
    /** Reserved for a platform capability that is not implemented. */
    CRNG_ERR_UNSUPPORTED = -4,
    /** A bounded search exhausted its attempt budget. */
    CRNG_ERR_NOT_FOUND = -5
} crng_status;

/**
 * Return a static, human-readable description for a status code.
 * Unknown values map to a stable fallback string. The result must not be freed.
 */
CRNG_API const char *crng_status_string(crng_status status);

#ifdef __cplusplus
}
#endif

#endif /* CLASSICAL_RNG_COMMON_H */
