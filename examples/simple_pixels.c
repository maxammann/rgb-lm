#include <stdio.h>
#include <assert.h>
#include <lm.h>

int main() {
    int x,y;
    printf("Starting\n");
    lm_gpio_init();

    uint32_t raw = lm_io_bits_new();
    uint32_t outputs = lm_gpio_init_output(raw);
    assert(outputs == raw);

    lmLedMatrix *matrix = lm_matrix_new(32, 32, 1);
    lm_matrix_clear(matrix);


    for (x = 0; x < 32; ++x) {
        for (y = 0; y < 32; ++y) {
            lm_matrix_set_pixel(matrix, x, y, 0, 0, 255);
        }
    }


    lmThread *thread = lm_thread_new(matrix);
    lm_thread_start(thread);

    lm_thread_wait(thread);

    lm_matrix_free(matrix);
    lm_thread_free(thread);
    return 0;
}