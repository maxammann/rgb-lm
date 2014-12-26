#include <stdio.h>
#include <assert.h>
#include <lm.h>

int main() {
    printf("Starting\n");
    lm_gpio_init();

    uint32_t raw = lm_io_bits_new();
    uint32_t outputs = lm_gpio_init_output(raw);
    assert(outputs == raw);

    lmLedMatrix *matrix = lm_matrix_new(32, 32);
    lm_matrix_clear(matrix);
    lm_matrix_fill(matrix, 255, 0, 0);

    lmThread *thread = lm_thread_new(matrix);
    lm_thread_start(thread);

    lm_thread_wait(thread);
    return 0;
}
