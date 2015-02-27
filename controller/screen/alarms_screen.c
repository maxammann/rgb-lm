#include <stdio.h>
#include "alarms_screen.h"
#include "screen.h"
#include "../alarms.h"
#include "../controller.h"

void alarms_screen(lmLedMatrix *matrix, long int elapsed) {
    lm_matrix_clear(matrix);

    rgb color = {0, 0, 255};

    lmString *str = lm_fonts_string_new();

//    printf("Alarm: %s\n", get_name(get_last_alarm()));

    lm_fonts_populate_string(get_font_library(), str, get_name(get_last_alarm()), get_default_font());

    int width = lm_fonts_string_width(str);
    int height = lm_fonts_string_height(str);


    lm_fonts_render_string(matrix, str, 16 - (width / 2), 16 - (height / 2), &color);
    lm_matrix_swap_buffers(matrix);
};

void register_alarms_screens() {
    register_screen("alarms", (screen_t) &alarms_screen);
}
