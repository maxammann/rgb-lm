#include "mesmerizing.h"
#include <math.h>
#include <stdio.h>

#define NUM_BALL 24

static rgb color = {204, 0, 204};

static double t;

double getY(double_t x, double t) {
    double int_part;
    double mod = modf((t * (x / 500. + 0.02)) / 2, &int_part);
    return 32. * sin(mod * M_PI);
}

void mesmerizing_menu_screen(lmLedMatrix *matrix, int16_t x, int16_t y, double elapsed, void *user_data) {
    t += elapsed * 10.0;

    int i;

    for (i = 0; i < NUM_BALL; ++i) {
        uint16_t y_ball = (uint16_t) getY(i, t);
        uint16_t x_ball = (uint16_t) (3 * i);

        lm_matrix_set_pixel(matrix, x + x_ball, y + y_ball, &color);
    }
}
