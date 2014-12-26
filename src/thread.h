#pragma once

#include "led-matrix.h"

typedef struct lmThread_ lmThread;

lmThread *lm_thread_new(lmLedMatrix *matrix);

void lm_thread_free(lmThread *thread);

void lm_thread_start(lmThread *thread);

void lm_thread_wait(lmThread *thread);
