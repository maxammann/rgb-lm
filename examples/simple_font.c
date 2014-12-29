#include <stdio.h>
#include <assert.h>
#include <lm.h>

int main() {
    rgb color = {255, 0, 255};

    printf("Starting fonts\n");
    lm_gpio_init();

    uint32_t raw = lm_io_bits_new();
    uint32_t outputs = lm_gpio_init_output(raw);
    assert(outputs == raw);

    lmLedMatrix *matrix = lm_matrix_new(32, 32, 11);
    lm_matrix_clear(matrix);

    lm_fonts_init();
    lmFont *font = lm_fonts_font_new("../fonts/arial_uni.ttf", 20);

    lm_fonts_print_string(matrix, "Fuck", font, 0, 2, color);
    lm_fonts_print_wstring(matrix, L"❤", font, 7, 16, color);

    lmString *fuck = lm_fonts_string_new();
    lm_fonts_populate_string(fuck, "Fuck", font);


    lmMatrix m = {
            2, 0,
            0, 2
    };

    lm_fonts_string_apply_transformation(fuck, m);

    lm_fonts_render_string(matrix, fuck, 0, 32, color);

    lm_fonts_string_free(fuck);

    lm_fonts_font_free(font);

    lm_matrix_swap_buffers(matrix);

    lmThread *thread = lm_thread_new(matrix);
    lm_thread_start(thread);

    lm_thread_wait(thread);

//    sleep(5);

    lm_thread_free(thread);
    lm_matrix_free(matrix);

    lm_fonts_free();
    return 0;
}
