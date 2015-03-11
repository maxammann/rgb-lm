#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "stdint.h"
#include "led-matrix.h"
#include "math.h"

struct lmLedMatrix_ {
    uint16_t columns, rows;
    uint8_t row_mask;
    uint8_t pwm_bits;
    io_bits *bitplane_buffer;

    int correct_luminance;

    pthread_mutex_t buffer_mutex;
    long unsigned int bitplane_size;
    io_bits *hot_bitplane_buffer;
};

uint16_t *luminance_table = NULL;

lmLedMatrix *lm_matrix_new(uint16_t columns, uint16_t rows, uint8_t pwm_bits) {
    return lm_matrix_new_lum(columns, rows, pwm_bits, 1);
}

lmLedMatrix *lm_matrix_new_lum(uint16_t columns, uint16_t rows, uint8_t pwm_bits, int correct_luminance) {
    lmLedMatrix *matrix = malloc(sizeof(lmLedMatrix));

    matrix->columns = columns;
    matrix->rows = rows;
    uint8_t double_rows = lm_matrix_double_rows(matrix);
    matrix->row_mask = double_rows - (uint8_t) 1;
    matrix->pwm_bits = pwm_bits;
    matrix->bitplane_size = sizeof(io_bits) * double_rows * columns * MAX_BITPLANES;
    matrix->hot_bitplane_buffer = calloc(1, matrix->bitplane_size);
    matrix->bitplane_buffer = calloc(1, matrix->bitplane_size);
    matrix->correct_luminance = correct_luminance;
    pthread_mutex_init(&matrix->buffer_mutex, NULL);

    return matrix;
}

void lm_matrix_free(lmLedMatrix *matrix) {
    pthread_mutex_destroy(&matrix->buffer_mutex);
    free(matrix->hot_bitplane_buffer);
    free(matrix->bitplane_buffer);
    free(matrix);
}

uint16_t lm_matrix_columns(lmLedMatrix *matrix) {
    return matrix->columns;
}

uint16_t lm_matrix_rows(lmLedMatrix *matrix) {
    return matrix->rows;
}

void lm_matrix_set_correct_luminance(lmLedMatrix *matrix, int value) {
    matrix->correct_luminance = value;
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

static uint16_t luminance_cie1931(uint8_t c) {
    float factor = ((1 << MAX_BITPLANES) - 1);
    double v = c * 100.0 / 255.0;
    return (uint16_t) (factor * ((v <= 8) ? v / 902.3 : pow((v + 16) / 116.0, 3)));
}

static void create_luminance_cie1931_table() {
    if (luminance_table != NULL) {
        return;
    }

    luminance_table = malloc(sizeof(uint16_t) * 256);

    uint16_t i;
    for (i = 0; i < 256; ++i)
        luminance_table[i] = luminance_cie1931((uint8_t) i);
}

static inline uint16_t map_color(lmLedMatrix *matrix, uint16_t color) {
    if (matrix->correct_luminance) {
        create_luminance_cie1931_table();

        return luminance_table[color];
    }

    return color << COLOR_SHIFT;
}

void lm_matrix_fill(lmLedMatrix *matrix, rgb *rgb) {
    int i, row, col;
    io_bits *bitplane = matrix->hot_bitplane_buffer;
    uint16_t columns = matrix->columns;

    const uint16_t red = map_color(matrix, rgb->r);
    const uint16_t green = map_color(matrix, rgb->g);
    const uint16_t blue = map_color(matrix, rgb->b);

    uint8_t double_rows = lm_matrix_double_rows(matrix);

    for (i = COLOR_SHIFT + MAX_BITPLANES - matrix->pwm_bits; i < MAX_BITPLANES; ++i) {
        int mask = 1 << i;

        int r = (red & mask) == mask;
        int b = (blue & mask) == mask;
        int g = (green & mask) == mask;

        io_bits plane_bits = { 0 };
        plane_bits.bits.r1 = plane_bits.bits.r2 = (bits_t) r;
        plane_bits.bits.g1 = plane_bits.bits.g2 = (bits_t) g;
        plane_bits.bits.b1 = plane_bits.bits.b2 = (bits_t) b;
        for (row = 0; row < double_rows; ++row) {
            io_bits *row_data = lm_io_bits_value_at(bitplane, columns, row, 0, i);
            for (col = 0; col < columns; ++col) {
                (row_data++)->raw = plane_bits.raw;
            }
        }
    }
}

void lm_matrix_set_pixel(lmLedMatrix *matrix,
        int16_t x, int16_t y,
        rgb *rgb) {
    if (x < 0 || y < 0
            || x >= matrix->columns || y >= matrix->rows) {
        return;
    }

    int i;

    uint16_t red = map_color(matrix, rgb->r);
    uint16_t green = map_color(matrix, rgb->g);
    uint16_t blue = map_color(matrix, rgb->b);

    uint8_t pwm = matrix->pwm_bits;
    uint16_t columns = matrix->columns;
    uint8_t double_rows = lm_matrix_double_rows(matrix);

    const int min_bit_plane = COLOR_SHIFT + MAX_BITPLANES - pwm;


    io_bits *bits = lm_io_bits_value_at(matrix->hot_bitplane_buffer, matrix->columns, y & matrix->row_mask, x, min_bit_plane);
    if (y < double_rows) {   // Upper sub-panel.
        for (i = min_bit_plane; i < MAX_BITPLANES; ++i) {
            int mask = 1 << i;

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
}

int sgn(int x) {
    return (x > 0) ? 1 : (x < 0) ? -1 : 0;
}

void lm_matrix_line(lmLedMatrix *matrix,
        int16_t xstart, int16_t ystart,
        int16_t xend, int16_t yend,
        rgb *rgb) {
    int x, y, t, dx, dy, incx, incy, pdx, pdy, ddx, ddy, es, el, err;

/* Entfernung in beiden Dimensionen berechnen */
    dx = xend - xstart;
    dy = yend - ystart;

/* Vorzeichen des Inkrements bestimmen */
    incx = sgn(dx);
    incy = sgn(dy);
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

/* feststellen, welche Entfernung größer ist */
    if (dx > dy) {
        /* x ist schnelle Richtung */
        pdx = incx;
        pdy = 0;    /* pd. ist Parallelschritt */
        ddx = incx;
        ddy = incy; /* dd. ist Diagonalschritt */
        es = dy;
        el = dx;   /* Fehlerschritte schnell, langsam */
    } else {
        /* y ist schnelle Richtung */
        pdx = 0;
        pdy = incy; /* pd. ist Parallelschritt */
        ddx = incx;
        ddy = incy; /* dd. ist Diagonalschritt */
        es = dx;
        el = dy;   /* Fehlerschritte schnell, langsam */
    }

/* Initialisierungen vor Schleifenbeginn */
    x = xstart;
    y = ystart;
    err = el / 2;
    lm_matrix_set_pixel(matrix, x, y, rgb);

/* Pixel berechnen */
    for (t = 0; t < el; ++t) /* t zaehlt die Pixel, el ist auch Anzahl */
    {
        /* Aktualisierung Fehlerterm */
        err -= es;
        if (err < 0) {
            /* Fehlerterm wieder positiv (>=0) machen */
            err += el;
            /* Schritt in langsame Richtung, Diagonalschritt */
            x += ddx;
            y += ddy;
        } else {
            /* Schritt in schnelle Richtung, Parallelschritt */
            x += pdx;
            y += pdy;
        }
        lm_matrix_set_pixel(matrix, x, y, rgb);
    }
}

void lm_matrix_clear(lmLedMatrix *matrix) {
    io_bits *bits = matrix->hot_bitplane_buffer;
    uint8_t double_rows = lm_matrix_double_rows(matrix);

    memset(bits, 0, sizeof(*bits) * double_rows * matrix->columns * MAX_BITPLANES);
}

void lm_matrix_swap_buffers(lmLedMatrix *matrix) {
    lm_matrix_lock(matrix);
    memcpy(matrix->bitplane_buffer, matrix->hot_bitplane_buffer, matrix->bitplane_size);
    lm_matrix_unlock(matrix);
}
