#include "time_util.h"
#include <math.h>

unsigned long current_time() {
    struct timespec spec;

    clock_gettime(CLOCK_MONOTONIC_RAW, &spec);

    return spec.tv_sec * 1000 + (long) round(spec.tv_nsec / 1.0e6);
}

inline double diff_nseconds(struct timespec a, struct timespec b) {
    return fabs(a.tv_sec * 10E9 + a.tv_nsec - b.tv_sec * 10E9 + b.tv_nsec);
}
