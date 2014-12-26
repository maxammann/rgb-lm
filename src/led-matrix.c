#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "stdint.h"
#include "led-matrix.h"

#ifdef INVERSE_RGB_DISPLAY_COLORS
#  define COLOR_OUT_BITS(x) (x) ^ 0xffff
#else
#  define COLOR_OUT_BITS(x) (x)
#endif

lmLedMatrix *lm_matrix_new(uint16_t columns, uint16_t rows, uint8_t pwm_bits) {
    uint8_t double_rows = (uint8_t) (rows / 2);

    lmLedMatrix *matrix = malloc(sizeof(lmLedMatrix));

    matrix->columns = columns;
    matrix->rows = rows;
    matrix->double_rows = double_rows;
    matrix->row_mask = double_rows - (uint8_t) 1;
    matrix->pwm_bits = pwm_bits;
    matrix->bitplane_buffer = calloc(1, sizeof(io_bits) * double_rows * columns * MAX_BITPLANES);
    pthread_mutex_init(&matrix->buffer_mutex, NULL);

    return matrix;
}

void lm_matrix_free(lmLedMatrix *matrix) {
    free(matrix);
}

uint16_t lm_matrix_columns(lmLedMatrix *matrix) {
    return matrix->columns;
}

uint16_t lm_matrix_rows(lmLedMatrix *matrix) {
    return matrix->rows;
}

uint16_t lm_matrix_pwm_bits(lmLedMatrix *matrix) {
    return matrix->pwm_bits;
}

static uint16_t map_color(uint16_t color) {
    enum {
        shift = MAX_BITPLANES - 8
    };

    return COLOR_OUT_BITS((shift > 0) ? (color << shift) : (color >> -shift));
}

//bitplanes code took from hzeller! https://github.com/hzeller/rpi-rgb-led-matrix/blob/440549553d58157cd3355b92fb791bf25f526fbd/lib/framebuffer.cc#L150
void lm_matrix_fill(lmLedMatrix *matrix, uint8_t r, uint8_t g, uint8_t b) {
    int i, row, col;

    const uint16_t red = map_color(r);
    const uint16_t green = map_color(g);
    const uint16_t blue = map_color(b);

    for (i = MAX_BITPLANES - matrix->pwm_bits; i < MAX_BITPLANES; ++i) {
        int mask = 1 << i;
        io_bits plane_bits;
        plane_bits.raw = 0;
        plane_bits.bits.r1 = plane_bits.bits.r2 = (bits_t) ((red & mask) == mask);
        plane_bits.bits.g1 = plane_bits.bits.g2 = (bits_t) ((green & mask) == mask);
        plane_bits.bits.b1 = plane_bits.bits.b2 = (bits_t) ((blue & mask) == mask);
        for (row = 0; row < matrix->double_rows; ++row) {
            pthread_mutex_lock(&matrix->buffer_mutex);
            io_bits *row_data = lm_io_bits_value_at(matrix->bitplane_buffer, matrix->columns, row, 0, i);
            for (col = 0; col < matrix->columns; ++col) {
                (row_data++)->raw = plane_bits.raw;
            }
            pthread_mutex_unlock(&matrix->buffer_mutex);
        }
    }
}

//bitplanes code took from hzeller! https://github.com/hzeller/rpi-rgb-led-matrix/blob/440549553d58157cd3355b92fb791bf25f526fbd/lib/framebuffer.cc#L171
void lm_matrix_set_pixel(lmLedMatrix *matrix,
        uint16_t x, uint16_t y,
        uint8_t r, uint8_t g, uint8_t b) {
    int i;

    uint16_t red = map_color(r);
    uint16_t green = map_color(g);
    uint16_t blue = map_color(b);

    uint8_t pwm = matrix->pwm_bits;
    uint16_t columns = matrix->columns;

    const int min_bit_plane = MAX_BITPLANES - pwm;

    pthread_mutex_lock(&matrix->buffer_mutex);
    io_bits *bits = lm_io_bits_value_at(matrix->bitplane_buffer, matrix->columns, y & matrix->row_mask, x, min_bit_plane);
    if (y < matrix->double_rows) {   // Upper sub-panel.
        for (i = min_bit_plane; i < MAX_BITPLANES; ++i) {
            const uint16_t mask = (uint16_t) 1 << i;

            bits->bits.r1 = (bits_t) ((red & mask) == mask);
            bits->bits.g1 = (bits_t) ((green & mask) == mask);
            bits->bits.b1 = (bits_t) ((blue & mask) == mask);
            bits += columns;
        }
    } else {
        for (i = min_bit_plane; i < MAX_BITPLANES; ++i) {
            int mask = 1 << i;
            bits->bits.r2 = (bits_t) ((red & mask) == mask);
            bits->bits.g2 = (bits_t) ((green & mask) == mask);
            bits->bits.b2 = (bits_t) ((blue & mask) == mask);
            bits += columns;
        }
    }
    pthread_mutex_unlock(&matrix->buffer_mutex);
}

void lm_matrix_clear(lmLedMatrix *matrix) {
    io_bits *bits = matrix->bitplane_buffer;
    pthread_mutex_lock(&matrix->buffer_mutex);
    memset(bits, 0, sizeof(*bits) * matrix->double_rows * matrix->columns * MAX_BITPLANES);
    pthread_mutex_unlock(&matrix->buffer_mutex);
}

