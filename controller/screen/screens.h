#pragma once

#include <lm/lm.h>
#include <math.h>
#include "screen.h"

#define NUM_BALL 24

#define TO_RADIANS(x) (double)x / 180.0 * M_PI

long int t;

inline double getY(double_t x, double_t t) {
    double int_part;
    double mod = modf((t * (x / 500. + 0.02)) / 2, &int_part);
    return 32. * sin(mod * M_PI);
}

void mesmerizing_screen(long int elapsed) {
    t += 1;
    int i;
    rgb color = {68 / 2, 73 / 2, 219 / 2};

    lm_matrix_clear(get_matrix());

    for (i = 0; i < NUM_BALL; ++i) {
        uint16_t y = (uint16_t) getY(i, t);
        uint16_t x = (uint16_t) (3 * i);

        lm_matrix_set_pixel(get_matrix(), x, y, &color);
    }

    lm_matrix_swap_buffers(get_matrix());
}

void matrix_vector_mult(double mat[][2], double *vec, double *result, int m, int n) {
    int i, j;
    double s;
    double *Ai;
    for (i = 0; i < n; i++) {
        s = 0.0;
        Ai = mat[i]; // requires a single access to the i-th row
        for (j = 0; j < m; j++) {
            s += Ai[j] * vec[j];
        }
        result[i] = s; // requires a single access of element c[i]
    }
}

void clock_screen(long int elapsed) {
    lm_matrix_clear(get_matrix());


    rgb color = {68 / 2, 73 / 2, 219 / 2};

    time_t rawtime;
    time(&rawtime);
    struct tm *tm_struct = localtime(&rawtime);
    int second = tm_struct->tm_sec;

    double angle = TO_RADIANS(360.0 * ((float) second / 60.0));

    // shift origin
    int16_t delta_x = 16;
    int16_t delta_y = 16;

    double v[] = {0, 16};

    double rotation[2][2] = {
            {cos(angle), -sin(angle)},
            {sin(angle), cos(angle)}
    };

    // Rotate vector v
    double result[2];
    matrix_vector_mult(rotation, v, result, 2, 2);


    double target_x = result[0];
    double target_y = result[1];

//    lm_matrix_line(get_matrix(), (uint16_t) delta_x, (uint16_t) delta_y, (uint16_t) (delta_x + target_x), (uint16_t) (delta_y + target_y), &color);


    // Catching 180° and 360° cases
    if ((int) target_x == 0) {
        int16_t i;

        // Start with smaller, end with greater
        int16_t start = (int16_t) (target_x > target_y ? target_y : target_x);
        int16_t end = (int16_t) (target_x > target_y ? target_x : target_y);

        for (i = start; i < end; ++i) {
            lm_matrix_set_pixel(get_matrix(), delta_x, delta_y + -i, &color);
        }

    } else {

        // Decide whether to draw on the left side of the clock or right side
        int left = (target_x < 0 && target_y < 0) || (target_x < 0 && target_y >= 0);

        // Find start points
        int16_t start = (int16_t) (left ? -16 : 0);
        int16_t end = (int16_t) (left ? 0 : 16);

        double m = target_y / target_x;

        int x;
        for (x = start; x < end; ++x) {
            // Calculate y position
            double y = m * x;

            lm_matrix_set_pixel(get_matrix(), (uint16_t) (delta_x + x), (uint16_t) (delta_y + y), &color);
        }
    }

    lm_matrix_swap_buffers(get_matrix());
};


void register_screens() {
    register_screen("clock", &clock_screen);
    register_screen("mesmerizing", &mesmerizing_screen);
}
