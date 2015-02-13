#include <stdio.h>
#include <assert.h>
#include <lm/lm.h>
#include "math.h"

int main() {
    rgb color = {255, 255, 0};

    printf("Starting fonts\n");
    lm_gpio_init();

    uint32_t raw = lm_io_bits_new();
    uint32_t outputs = lm_gpio_init_output(raw);
    assert(outputs == raw);

    lmLedMatrix *matrix = lm_matrix_new(32, 32, 11);


    lmFontLibrary *library = lm_fonts_init();

    lmFont *font = lm_fonts_font_new(library, "/root/projects/fonts/arialbd.ttf", 16);

    lmThread *thread = lm_thread_new(matrix, DEFAULT_BASE_TIME_NANOS);
    lm_thread_start(thread);


    lmString *string = lm_fonts_string_new();

    lm_fonts_populate_string(library, string, "Fuck", font);


    double angle = 45.0 / 180.0 * M_PI;
    lmMatrix m;
    m.xx = cos(angle) * 2;
    m.xy = sin(angle);
    m.yx = -sin(angle);
    m.yy = cos(angle) * 2;

    lm_fonts_string_apply_transformation(string, m);


    lm_fonts_render_string(matrix, string, 16, 16, &color);

    lm_fonts_font_free(library, font);

    lm_matrix_swap_buffers(matrix);




//    usleep(3* 1000000);

    lm_thread_wait(thread);
//    lm_thread_unpause(thread);


    lm_thread_stop(thread);
    lm_thread_free(thread);
    lm_matrix_free(matrix);

    lm_fonts_free(library);
    return 0;
}
