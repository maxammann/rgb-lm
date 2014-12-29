#include <stdio.h>
#include <assert.h>
#include <lm/lm.h>

int main() {
    rgb color = {255, 0, 0};

    printf("Starting\n");
    lm_gpio_init();

    uint32_t raw = lm_io_bits_new();
    uint32_t outputs = lm_gpio_init_output(raw);
    assert(outputs == raw);

    lmLedMatrix *matrix = lm_matrix_new(32, 32, 1);
    lm_matrix_clear(matrix);
    lm_matrix_fill(matrix, color);
    lm_matrix_swap_buffers(matrix);

    lmThread *thread = lm_thread_new(matrix, DEFAULT_BASE_TIME_NANOS);
    lm_thread_start(thread);

    lm_thread_wait(thread);

    lm_matrix_free(matrix);
    lm_thread_free(thread);
    return 0;
}
