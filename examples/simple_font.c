#include <stdio.h>
#include <assert.h>
#include <lm.h>

int main() {
    printf("Starting fonts\n");
    lm_gpio_init();

    uint32_t raw = lm_io_bits_new();
    uint32_t outputs = lm_gpio_init_output(raw);
    assert(outputs == raw);

    lmLedMatrix *matrix = lm_matrix_new(32, 32, 11);
    lm_matrix_clear(matrix);

    printf("Fonts: %d\n", lm_matrix_font_init());
    lmFont *font = lm_matrix_font_create("../fonts/PressStart2P.ttf", 10);

    lm_matrix_print_string(matrix, "Fuck", font, 0, 2, 255, 0, 0);
    lm_matrix_print_wstring(matrix, L"❤", font, 7, 16, 255, 0, 0);

    lm_matrix_font_destroy(font);

    lmThread *thread = lm_thread_new(matrix);
    lm_thread_start(thread);

//    lm_thread_wait(thread);

    sleep(5);

    lm_thread_free(thread);
    lm_matrix_free(matrix);

    lm_matrix_font_free();
    return 0;
}
