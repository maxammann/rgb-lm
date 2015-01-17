#pragma once

#include <lm/lm.h>
#include <math.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "screen.h"
#include "../controller.h"

#define NUM_BALL 24

#define TO_RADIANS(x) (double)x / 180.0 * M_PI

long int t;

double getY(double_t x, double_t t) {
    double int_part;
    double mod = modf((t * (x / 500. + 0.02)) / 2, &int_part);
    return 32. * sin(mod * M_PI);
}

void mesmerizing_screen(lmLedMatrix *matrix, long int elapsed) {
    t += 1;
    int i;
    rgb color = {68 / 2, 73 / 2, 219 / 2};

    lm_matrix_clear(matrix);

    for (i = 0; i < NUM_BALL; ++i) {
        uint16_t y = (uint16_t) getY(i, t);
        uint16_t x = (uint16_t) (3 * i);

        lm_matrix_set_pixel(matrix, x, y, &color);
    }

    lm_matrix_swap_buffers(matrix);
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

void draw_hand(lmLedMatrix *matrix, float t, double length, rgb *color) {
    double angle = TO_RADIANS(360.0 * t);

    // shift origin
    int16_t delta_x = 16;
    int16_t delta_y = 16;

    double v[] = {0, length};

    double rotation[2][2] = {
            {cos(angle), -sin(angle)},
            {sin(angle), cos(angle)}
    };

    // Rotate vector v
    double result[2];
    matrix_vector_mult(rotation, v, result, 2, 2);


    double target_x = result[0];
    double target_y = result[1];

    lm_matrix_line(matrix, (uint16_t) delta_x, (uint16_t) delta_y, (uint16_t) (delta_x + target_x), (uint16_t) (delta_y + target_y), color);
}

void clock_screen(lmLedMatrix *matrix, long int elapsed) {
    lm_matrix_clear(matrix);

    rgb color = {68 / 2, 73 / 2, 219 / 2};

    time_t rawtime;
    time(&rawtime);
    struct tm *tm_struct = localtime(&rawtime);
    float hour = tm_struct->tm_hour;
    float minute = tm_struct->tm_min;
//    float second = tm_struct->tm_sec;

    draw_hand(matrix, hour / 24, 12, &color);
    draw_hand(matrix, minute / 60, 16, &color);
//    draw_hand(second / 60, 16, &color);

    lm_matrix_swap_buffers(matrix);
};

void digital_clock_screen(lmLedMatrix *matrix, long int elapsed) {
    lm_matrix_clear(matrix);

    rgb color = {68, 73, 219};

    uint16_t delta_x = 16;
    uint16_t delta_y = 2;
    uint16_t spacing_y = 4;

    time_t rawtime;
    time(&rawtime);
    struct tm *tm_struct = localtime(&rawtime);

    char *time_hour;
    char *time_minute;

    // Hex representation
    time_hour = g_strdup_printf("%X", tm_struct->tm_hour);
    time_minute = g_strdup_printf("%X", tm_struct->tm_min);
//    time_hour = g_strdup_printf("%i", tm_struct->tm_hour);
//    time_minute = g_strdup_printf("%i", tm_struct->tm_min);

    lmString *hour = lm_fonts_string_new();
    lmString *minute = lm_fonts_string_new();

    lm_fonts_populate_string(get_font_library(), hour, time_hour, get_default_font());
    lm_fonts_populate_string(get_font_library(), minute, time_minute, get_default_font());

    int hour_width = lm_fonts_string_width(hour);
    int minute_width = lm_fonts_string_width(minute);
    int hour_height = lm_fonts_string_height(hour);

    lm_fonts_render_string(matrix, hour, (uint16_t) (delta_x - hour_width / 2), delta_y, &color);
    lm_fonts_render_string(matrix, minute, (uint16_t) (delta_x - minute_width / 2), (uint16_t) (delta_y + hour_height + spacing_y), &color);

    lm_matrix_swap_buffers(matrix);

    lm_fonts_string_free(hour);
    lm_fonts_string_free(minute);
};

#define MENUES 3

#define NEW_MENU(i, text)  lm_fonts_populate_wstring(get_font_library(), menus[i], text, get_default_font());
#define NEW_LMENU(i, text, len) lm_fonts_populate_lstring(get_font_library(), menus[i], text, len, get_default_font());

lmString **menus;

int current_menu = 1;


void menu_screen_init() {
    int i;

    menus = malloc(MENUES * sizeof(lmString *));

    for (i = 0; i < MENUES; ++i) {
        menus[i] = lm_fonts_string_new();
    }

    NEW_MENU(0, L"⏰");
    NEW_MENU(1, L"☁");
    unsigned long speaker = 0x1F50A;
    NEW_LMENU(2, &speaker, 1);
}

void menu_screen(lmLedMatrix *matrix, long int elapsed) {
    lm_matrix_clear(matrix);

    rgb color = {0, 0, 255};

    lmString *menu = menus[current_menu];

    int width = lm_fonts_string_width(menu);
    int height = lm_fonts_string_height(menu);

//    lmMatrix m;
//    m.xx = 2;
//    m.xy = 0;
//    m.yx =0;
//    m.yy = 2;
//
//    lm_fonts_string_apply_transformation(menu, m);

    lm_fonts_render_string(matrix, menu, 16 - (width / 2), 16 - (height / 2), &color);
    lm_matrix_swap_buffers(matrix);
};

void register_screens() {
    register_screen("digital_clock", (screen_t) &digital_clock_screen);
    register_screen("clock", (screen_t) &clock_screen);
    register_screen("mesmerizing", (screen_t) &mesmerizing_screen);
    menu_screen_init();
    register_screen("menu", (screen_t) &menu_screen);
}
