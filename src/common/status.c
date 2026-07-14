#include "classical_rng/common.h"

const char *crng_status_string(crng_status status) {
    switch (status) {
        case CRNG_OK:
            return "success";
        case CRNG_ERR_NULL:
            return "null pointer";
        case CRNG_ERR_INVALID_RANGE:
            return "invalid range";
        case CRNG_ERR_SYSTEM:
            return "operating-system random source failed";
        case CRNG_ERR_UNSUPPORTED:
            return "platform is not supported";
        case CRNG_ERR_NOT_FOUND:
            return "no matching value found within the attempt limit";
        default:
            return "unknown classical_rng status";
    }
}
