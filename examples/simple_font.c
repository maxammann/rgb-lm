#include <stdio.h>
#include <assert.h>
#include <lm/lm.h>
#include <unistd.h>

int main() {
    rgb color = {255, 255, 0};

    printf("Starting fonts\n");
    lm_gpio_init();

    uint32_t raw = lm_io_bits_new();
    uint32_t outputs = lm_gpio_init_output(raw);
    assert(outputs == raw);

    lmLedMatrix *matrix = lm_matrix_new(32, 32, 11);


    lmFontLibrary *library = lm_fonts_init();

    lmFont *font = lm_fonts_font_new(library, "/root/projects/rgb-led-matrix/fonts/arialbd.ttf", 16);

    lmThread *thread = lm_thread_new(matrix, DEFAULT_BASE_TIME_NANOS);
    lm_thread_start(thread);


    lm_fonts_print_string(library, matrix, "Pierre", font, 0, 0, &color);

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
