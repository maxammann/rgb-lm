#include <stdio.h>
#include <assert.h>
#include <lm/lm.h>
#include <unistd.h>

#include "math.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#define NUM_BALL 24

inline double getY(double_t i, double_t t) {
    double int_part;
    double mod = modf((t * (i / 500. + 0.02)) / 2, &int_part);
    return 32. * sin(mod * M_PI);
}

int main() {
    int i, t = 0;
    rgb color = {0, 0, 255};

    lm_gpio_init();

    uint32_t raw = lm_io_bits_new();
    uint32_t outputs = lm_gpio_init_output(raw);
    assert(outputs == raw);

    lmLedMatrix *matrix = lm_matrix_new(32, 32, 11);
    lm_matrix_clear(matrix);

    lmThread *thread = lm_thread_new(matrix, DEFAULT_BASE_TIME_NANOS);
    lm_thread_start(thread);

    uint32_t continuum = 0;

    while (1) {
        lm_matrix_clear(matrix);

        continuum++;

        continuum %= 3 * 255;
        int r = 0, g = 0, b = 0;
        if (continuum <= 255) {
            int c = continuum;
            b = 255 - c;
            r = c;
        } else if (continuum > 255 && continuum <= 511) {
            int c = continuum - 256;
            r = 255 - c;
            g = c;
        } else {
            int c = continuum - 512;
            g = 255 - c;
            b = c;
        }

        color.r = (uint8_t) (float) r * 0.5;
        color.g = (uint8_t) (float) g * 0.5;
        color.b = (uint8_t) (float) b * 0.5;

        for (i = 0; i < NUM_BALL; ++i) {
            uint16_t y = (uint16_t) getY(i, t);
            uint16_t x = (uint16_t) (3 * i);

            lm_matrix_set_pixel(matrix, y, x, &color);
            lm_matrix_set_pixel(matrix, y + 1, x, &color);
//            lm_matrix_set_pixel(matrix, x, y, &color);
//
//            lm_matrix_set_pixel(matrix, 32 - y, 32 - x, &color);
//            lm_matrix_set_pixel(matrix, 32 - x, 32 - y, &color);
        }

        t++;

        usleep(15000);

        lm_matrix_swap_buffers(matrix);
    }


    lm_matrix_free(matrix);
    lm_thread_free(thread);
    return 0;
}

#pragma clang diagnostic pop
