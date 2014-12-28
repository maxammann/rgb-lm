#pragma once

#include <pthread.h>
#include "stdint.h"
#include "io_bits.h"

typedef struct lmLedMatrix_ lmLedMatrix;

lmLedMatrix *lm_matrix_new(uint16_t columns, uint16_t rows, uint8_t pwm_bits);

void lm_matrix_free(lmLedMatrix *matrix);

uint16_t lm_matrix_columns(lmLedMatrix *matrix);

uint16_t lm_matrix_rows(lmLedMatrix *matrix);

uint16_t lm_matrix_pwm_bits(lmLedMatrix *matrix);

void lm_matrix_lock(lmLedMatrix *matrix);

void lm_matrix_unlock(lmLedMatrix *matrix);

inline uint8_t lm_matrix_double_rows(lmLedMatrix *matrix);

io_bits *lm_matrix_bit_plane(lmLedMatrix *matrix);

void lm_matrix_fill(lmLedMatrix *matrix,
        uint8_t r, uint8_t g, uint8_t b);

void lm_matrix_set_pixel(lmLedMatrix *matrix,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue);

void lm_matrix_print_string(lmLedMatrix *matrix, char *s, char *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue);


void lm_matrix_clear(lmLedMatrix *matrix);
