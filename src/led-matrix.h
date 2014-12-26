#pragma once

#include <pthread.h>
#include "stdint.h"
#include "io_bits.h"

typedef struct lmLedMatrix_ lmLedMatrix;

struct lmLedMatrix_ {
    uint16_t columns, rows;
    int double_rows;
    uint8_t row_mask;

    uint8_t pwm_bits;
    io_bits *bitplane_buffer;

    pthread_mutex_t buffer_mutex;
};


lmLedMatrix *lm_matrix_new(uint16_t columns, uint16_t rows);

void lm_matrix_free(lmLedMatrix *matrix);

uint16_t lm_matrix_columns(lmLedMatrix *matrix);

uint16_t lm_matrix_rows(lmLedMatrix *matrix);

uint16_t lm_matrix_pwm_bits(lmLedMatrix *matrix);

uint16_t lm_matrix_double_rows(lmLedMatrix *matrix);

void lm_matrix_fill(lmLedMatrix *matrix,
        uint8_t r, uint8_t g, uint8_t b);

void lm_matrix_set_pixel(lmLedMatrix *matrix,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue);

void lm_matrix_clear(lmLedMatrix *matrix);
