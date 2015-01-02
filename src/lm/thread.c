#include <pthread.h>
#include <stdlib.h>
#include "io_bits.h"
#include "thread.h"
#include "gpio.h"

struct lmThread_ {
    int stopped;
    volatile int halted;
    pthread_mutex_t halt_mutex;
    pthread_cond_t halt_cond;

    lmLedMatrix *matrix;

    pthread_t pthread;

    long row_sleep_timings[MAX_BITPLANES];
};


//Thanks to hzeller for these timings! https://github.com/hzeller/rpi-rgb-led-matrix/blob/440549553d58157cd3355b92fb791bf25f526fbd/lib/framebuffer.cc#L48 :D
static void sleep_nanos(long nanos) {

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

//bitplanes code took from hzeller! https://github.com/hzeller/rpi-rgb-led-matrix/blob/440549553d58157cd3355b92fb791bf25f526fbd/lib/framebuffer.cc#L200
static void *main(void *ch) {
    lmThread *thread = ch;

    long *sleep_timings = thread->row_sleep_timings;
    lmLedMatrix *matrix = thread->matrix;
    int double_rows = lm_matrix_double_rows(matrix);
    io_bits *bitplane = lm_matrix_bit_plane(matrix);
    uint16_t columns = lm_matrix_columns(matrix);

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

    while (!thread->stopped) {

        pthread_mutex_lock(&thread->halt_mutex);
        while (thread->halted) {
            pthread_cond_wait(&thread->halt_cond, &thread->halt_mutex);
        }
        pthread_mutex_unlock(&thread->halt_mutex);


        uint16_t pwm_bits = lm_matrix_pwm_bits(matrix);

        for (d_row = 0; d_row < double_rows; ++d_row) {
            row_address.bits.row = d_row;
            lm_gpio_set_masked_bits(row_address.raw, row_mask.raw);  // Set row address

            // Rows can't be switched very quickly without ghosting, so we do the
            // full PWM of one row before switching rows.
            for (b = MAX_BITPLANES - pwm_bits; b < MAX_BITPLANES; ++b) {

                lm_matrix_lock(matrix);
                io_bits *row_data = lm_io_bits_value_at(bitplane, columns, d_row, 0, b);
                // We clock these in while we are dark. This actually increases the
                // dark time, but we ignore that a bit.
                for (col = 0; col < columns; ++col) {
                    const io_bits out = *row_data++;
                    lm_gpio_set_masked_bits(out.raw, color_clk_mask.raw);  // col + reset clock
                    lm_gpio_set_bits(clock.raw);               // Rising edge: clock color in.
                }
                lm_matrix_unlock(matrix);

                lm_gpio_clear_bits(color_clk_mask.raw);    // clock back to normal.

                lm_gpio_set_bits(strobe.raw);   // Strobe in the previously clocked in row.
                lm_gpio_clear_bits(strobe.raw);

                // Now switch on for the sleep time necessary for that bit-plane.
                lm_gpio_clear_bits(output_enable.raw);
                sleep_nanos(sleep_timings[b]);
                lm_gpio_set_bits(output_enable.raw);
            }
        }
    }

    return NULL;
}

void lm_thread_start(lmThread *thread) {
    thread->stopped = 0;
    thread->halted = 0;

    pthread_t pthread;
    pthread_create(&pthread, NULL, main, thread);
    struct sched_param p;
    p.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(pthread, SCHED_FIFO, &p);
    thread->pthread = pthread;
}

void lm_thread_wait(lmThread *thread) {
    if (!thread->stopped) {
        pthread_join(thread->pthread, NULL);
    }
}

lmThread *lm_thread_new(lmLedMatrix *matrix, long base_time_nanos) {
    int i;

    lmThread *thread = malloc(sizeof(lmThread));
    thread->stopped = 0;
    thread->halted = 0;
    thread->matrix = matrix;

    for (i = 0; i < MAX_BITPLANES; ++i) {
        thread->row_sleep_timings[i] = (1 << i) * base_time_nanos;
    }
    return thread;
}

void lm_thread_free(lmThread *thread) {
    lm_thread_stop(thread);
    lm_thread_wait(thread);
    pthread_detach(thread->pthread);
    free(thread);
}

void lm_thread_pause(lmThread *thread) {
    pthread_mutex_lock(&thread->halt_mutex);
    thread->halted = 1;
    pthread_mutex_unlock(&thread->halt_mutex);
}

void lm_thread_unpause(lmThread *thread) {
    pthread_mutex_lock(&thread->halt_mutex);
    thread->halted = 0;
    pthread_cond_signal(&thread->halt_cond);
    pthread_mutex_unlock(&thread->halt_mutex);
}

void lm_thread_stop(lmThread *thread) {
    thread->stopped = 1;
}

int lm_thread_is_halted(lmThread *thread) {
    return thread->halted;
}

int lm_thread_is_stopped(lmThread *thread) {
    return thread->stopped;
}


