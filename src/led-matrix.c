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

#define COLOR_SHIRT MAX_BITPLANES - 8

struct lmLedMatrix_ {
    uint16_t columns, rows;
    uint8_t row_mask;

    uint8_t pwm_bits;
    io_bits *bitplane_buffer;

    pthread_mutex_t buffer_mutex;
};

lmLedMatrix *lm_matrix_new(uint16_t columns, uint16_t rows, uint8_t pwm_bits) {
    lmLedMatrix *matrix = malloc(sizeof(lmLedMatrix));

    matrix->columns = columns;
    matrix->rows = rows;
    uint8_t double_rows = lm_matrix_double_rows(matrix);
    matrix->row_mask = double_rows - (uint8_t) 1;
    matrix->pwm_bits = pwm_bits;
    matrix->bitplane_buffer = calloc(1, sizeof(io_bits) * double_rows * columns * MAX_BITPLANES);
    pthread_mutex_init(&matrix->buffer_mutex, NULL);

    return matrix;
}

void lm_matrix_free(lmLedMatrix *matrix) {
    pthread_mutex_destroy(&matrix->buffer_mutex);
    free(matrix->bitplane_buffer);
    free(matrix);
}

uint16_t lm_matrix_columns(lmLedMatrix *matrix) {
    return matrix->columns;
}

uint16_t lm_matrix_rows(lmLedMatrix *matrix) {
    return matrix->rows;
}

void lm_matrix_lock(lmLedMatrix *matrix) {
    pthread_mutex_lock(&matrix->buffer_mutex);
}

void lm_matrix_unlock(lmLedMatrix *matrix) {
    pthread_mutex_unlock(&matrix->buffer_mutex);
}

uint16_t lm_matrix_pwm_bits(lmLedMatrix *matrix) {
    return matrix->pwm_bits;
}

inline uint8_t lm_matrix_double_rows(lmLedMatrix *matrix) {
    return (uint8_t) (matrix->rows / 2);
}

io_bits *lm_matrix_bit_plane(lmLedMatrix *matrix) {
    return matrix->bitplane_buffer;
}

static uint16_t map_color(uint16_t color) {
    return COLOR_OUT_BITS((COLOR_SHIRT > 0) ? (color << COLOR_SHIRT) : (color >> -COLOR_SHIRT));
}

//bitplanes code took from hzeller! https://github.com/hzeller/rpi-rgb-led-matrix/blob/440549553d58157cd3355b92fb791bf25f526fbd/lib/framebuffer.cc#L150
void lm_matrix_fill(lmLedMatrix *matrix, uint8_t r, uint8_t g, uint8_t b) {
    int i, row, col;

    const uint16_t red = map_color(r);
    const uint16_t green = map_color(g);
    const uint16_t blue = map_color(b);

    uint8_t double_rows = lm_matrix_double_rows(matrix);

    for (i = MAX_BITPLANES - matrix->pwm_bits; i < MAX_BITPLANES; ++i) {
        int mask = 1 << i;
        io_bits plane_bits;
        plane_bits.raw = 0;
        plane_bits.bits.r1 = plane_bits.bits.r2 = (bits_t) ((red & mask) == mask);
        plane_bits.bits.g1 = plane_bits.bits.g2 = (bits_t) ((green & mask) == mask);
        plane_bits.bits.b1 = plane_bits.bits.b2 = (bits_t) ((blue & mask) == mask);
        for (row = 0; row < double_rows; ++row) {
            lm_matrix_lock(matrix);
            io_bits *row_data = lm_io_bits_value_at(matrix->bitplane_buffer, matrix->columns, row, 0, i);
            for (col = 0; col < matrix->columns; ++col) {
                (row_data++)->raw = plane_bits.raw;
            }
            lm_matrix_unlock(matrix);
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
    uint8_t double_rows = lm_matrix_double_rows(matrix);

    const int min_bit_plane = MAX_BITPLANES - pwm;

    lm_matrix_lock(matrix);
    io_bits *bits = lm_io_bits_value_at(matrix->bitplane_buffer, matrix->columns, y & matrix->row_mask, x, min_bit_plane);
    if (y < double_rows) {   // Upper sub-panel.
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
    lm_matrix_unlock(matrix);
}

void lm_matrix_clear(lmLedMatrix *matrix) {
    io_bits *bits = matrix->bitplane_buffer;
    uint8_t double_rows = lm_matrix_double_rows(matrix);

    lm_matrix_lock(matrix);
    memset(bits, 0, sizeof(*bits) * double_rows * matrix->columns * MAX_BITPLANES);
    lm_matrix_unlock(matrix);
}

