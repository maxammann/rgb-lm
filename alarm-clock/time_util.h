#pragma once

#include <time.h>

unsigned long current_time();

double diff_nseconds(struct timespec a, struct timespec b);