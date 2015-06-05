#include "clock.h"
#include "../../controller.h"
#include <stdio.h>

void digital_clock_menu_screen(lmLedMatrix *matrix, int16_t x, int16_t y, double elapsed, void *user_data) {
    rgb color = {255, 0, 0};

    int16_t delta_x = 16;
    int16_t delta_y = 1;
    int16_t spacing_y = 2;

    time_t rawtime;
    time(&rawtime);
    struct tm *tm_struct = localtime(&rawtime);

    char time_hour[10];
    char time_minute[10];

    sprintf(time_hour, "%i", tm_struct->tm_hour);
    sprintf(time_minute, "%i", tm_struct->tm_min);

    lmString *hour = lm_fonts_string_new();
    lmString *minute = lm_fonts_string_new();

    lm_fonts_populate_string(get_font_library(), hour, time_hour, get_default_font());
    lm_fonts_populate_string(get_font_library(), minute, time_minute, get_default_font());

    int hour_width = lm_fonts_string_width(hour);
    int minute_width = lm_fonts_string_width(minute);
    int hour_height = lm_fonts_string_height(hour);

//    rgb bg = {255, 255, 255};
//    int16_t x1, y1;
//
//    for (x1 = 0; x1 < 32; ++x1) {
//        for (y1 = 0; y1 < 32; ++y1) {
//
//            lm_matrix_set_pixel(matrix, x + x1, y + y1, &bg);
//        }
//    }

    lm_fonts_render_string(matrix, hour,
                           x + (delta_x - hour_width / 2),
                           y + delta_y,
                           &color
    );

    lm_fonts_render_string(matrix, minute,
                           x + (delta_x - minute_width / 2),
                           y + delta_y + hour_height + spacing_y,
                           &color
    );

    lm_fonts_string_free(hour);
    lm_fonts_string_free(minute);
};