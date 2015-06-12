#pragma once

#include "led-matrix.h"

#define DEFAULT_BASE_TIME_NANOS 200

typedef struct lmThread_ lmThread;

lmThread *lm_thread_new(lmLedMatrix *matrix, long base_time_nanos);

void lm_thread_free(lmThread *thread);

void lm_thread_start(lmThread *thread);

void lm_thread_wait(lmThread *thread);

int lm_thread_is_paused(lmThread *thread);

void lm_thread_pause(lmThread *thread);

void lm_thread_unpause(lmThread *thread);

void lm_thread_stop(lmThread *thread);

int lm_thread_is_stopped(lmThread *thread);

void sleep_nanos(long nanos);