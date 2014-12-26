#include <pthread.h>
#include <stdlib.h>
#include "io_bits.h"
#include "thread.h"
#include "gpio.h"
#include "stdio.h"

#define BASE_TIME_NANOS 200

struct lmThread_ {
    int running;
    lmLedMatrix *matrix;

    pthread_t pthread;
};

const long row_sleep_nanos[11] = {
        (1 * BASE_TIME_NANOS),
        (2 * BASE_TIME_NANOS),
        (4 * BASE_TIME_NANOS),
        (8 * BASE_TIME_NANOS),
        (16 * BASE_TIME_NANOS),
        (32 * BASE_TIME_NANOS),
        (64 * BASE_TIME_NANOS),
        (128 * BASE_TIME_NANOS),
        (256 * BASE_TIME_NANOS),
        (512 * BASE_TIME_NANOS),
        (1024 * BASE_TIME_NANOS),
};

static void sleep_nanos(long nanos) {
    // For sleep times above 20usec, nanosleep seems to be fine, but it has
    // an offset of about 20usec (on the RPi distribution I was testing it on).
    // That means, we need to give it 80us to get 100us.
    // For values lower than roughly 30us, this is not accurate anymore and we
    // need to switch to busy wait.
    // TODO: compile Linux kernel realtime extensions and watch if the offset-time
    // changes and hope for less jitter.
    if (nanos > 28000) {
        struct timespec sleep_time = {0, nanos - 20000};
        nanosleep(&sleep_time, NULL);
    } else {
        long i;

        // The following loop is determined empirically on a 700Mhz RPi //Thanks to hzeller :D
        for (i = nanos >> 2; i != 0; --i) {
            asm("");   // force GCC not to optimize this away.
        }
    }
}

static void *main(void *ch) {
    lmThread *thread = ch;
    lmLedMatrix *matrix = thread->matrix;
    uint16_t pwm_bits = matrix->pwm_bits;
    int double_rows = matrix->double_rows;
    io_bits *bitplane = matrix->bitplane_buffer;
    uint16_t columns = matrix->columns;

    uint8_t d_row;
    int b;
    int col;

    io_bits color_clk_mask;   // Mask of bits we need to set while clocking in.
    color_clk_mask.raw = 0;
    color_clk_mask.bits.r1 = color_clk_mask.bits.g1 = color_clk_mask.bits.b1 = 1;
    color_clk_mask.bits.r2 = color_clk_mask.bits.g2 = color_clk_mask.bits.b2 = 1;
    SET_CLOCK(color_clk_mask.bits, 1);

    io_bits row_mask;
    row_mask.raw = 0;
    row_mask.bits.row = 0x0f;

    io_bits clock, output_enable, strobe, row_address;
    clock.raw = output_enable.raw = strobe.raw = row_address.raw = 0;

    SET_CLOCK(clock.bits, 1);
    ENABLE_OUTPUT(output_enable.bits, 1);
    strobe.bits.strobe = 1;

    while (thread->running) {
        for (d_row = 0; d_row < double_rows; ++d_row) {
            row_address.bits.row = d_row;
            lm_gpio_set_masked_bits(row_address.raw, row_mask.raw);  // Set row address

            // Rows can't be switched very quickly without ghosting, so we do the
            // full PWM of one row before switching rows.
            for (b = MAX_BITPLANES - pwm_bits; b < MAX_BITPLANES; ++b) {

                pthread_mutex_lock(&matrix->buffer_mutex);

                io_bits *row_data = lm_io_bits_value_at(bitplane, columns, d_row, 0, b);
                // We clock these in while we are dark. This actually increases the
                // dark time, but we ignore that a bit.
                for (col = 0; col < columns; ++col) {
                    const io_bits out = *row_data++;
                    lm_gpio_set_masked_bits(out.raw, color_clk_mask.raw);  // col + reset clock
                    lm_gpio_set_bits(clock.raw);               // Rising edge: clock color in.
                }

                pthread_mutex_unlock(&matrix->buffer_mutex);

                lm_gpio_clear_bits(color_clk_mask.raw);    // clock back to normal.

                lm_gpio_set_bits(strobe.raw);   // Strobe in the previously clocked in row.
                lm_gpio_clear_bits(strobe.raw);

                // Now switch on for the sleep time necessary for that bit-plane.
                lm_gpio_clear_bits(output_enable.raw);
                sleep_nanos(row_sleep_nanos[b]);
                lm_gpio_set_bits(output_enable.raw);
            }
        }
    }

    return NULL;
}

void lm_thread_start(lmThread *thread) {
    thread->running = 1;

    pthread_t pthread;
    pthread_create(&pthread, NULL, main, thread);
    struct sched_param p;
    p.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(pthread, SCHED_FIFO, &p);

    printf("Priority: %d\n", p.sched_priority);

    thread->pthread = pthread;
}

void lm_thread_wait(lmThread *thread) {
    pthread_join(thread->pthread, NULL);
}

lmThread *lm_thread_new(lmLedMatrix *matrix) {
    lmThread *thread = malloc(sizeof(lmThread));
    thread->running = 0;
    thread->matrix = matrix;
    return thread;
}

void lm_thread_free(lmThread *thread) {
    free(thread);
}

