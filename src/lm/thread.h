#pragma once

#include "led-matrix.h"

#define DEFAULT_BASE_TIME_NANOS 200

typedef struct lmThread_ lmThread;

lmThread *lm_thread_new(lmLedMatrix *matrix, long base_time_nanos);

void lm_thread_free(lmThread *thread);

void lm_thread_start(lmThread *thread);

void lm_thread_wait(lmThread *thread);
