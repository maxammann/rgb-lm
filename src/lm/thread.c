#include <pthread.h>
#include <stdlib.h>
#include "io_bits.h"
#include "thread.h"
#include "gpio.h"

#ifdef RT

#define CITICAL_NANOS 21300     //The amount of nanos when we're using "busy wait"
#define BUSY_OPERATION / 3
#define NANO_SLEEP_OFFSET 21300

#else

#define CITICAL_NANOS 28000
#define BUSY_OPERATION >> 2
#define NANO_SLEEP_OFFSET 20000

#endif


struct lmThread_ {

    pthread_t pthread;
    pthread_mutex_t halt_mutex;
    pthread_cond_t halt_cond;

    int stopped;
    volatile int halted;
    lmLedMatrix *matrix;
    long row_sleep_timings[MAX_BITPLANES];
};


static void sleep_nanos(long nanos) {
    if (nanos > CITICAL_NANOS) {
        struct timespec sleep_time = {0, nanos - NANO_SLEEP_OFFSET};
        nanosleep(&sleep_time, NULL);
    } else {
        long i;

        for (i = nanos BUSY_OPERATION; i != 0; --i) {
            asm("");   // skip gcc
        }
    }
}

static void *main(void *ch) {

    uint8_t d_row;
    int b;
    int col;

    lmThread *thread = ch;
    long *sleep_timings = thread->row_sleep_timings;
    lmLedMatrix *matrix = thread->matrix;
    int double_rows = lm_matrix_double_rows(matrix);
    io_bits *bitplane = lm_matrix_bit_plane(matrix);
    uint16_t columns = lm_matrix_columns(matrix);


    io_bits color_clock_mask = {0};   // Mask of bits we need to set while clocking in.
    io_bits clock = {0}, output_enable = {0}, strobe = {0}, row_address = {0};
    io_bits row_mask = {0};

    // Color & clock
    color_clock_mask.bits.r1 = color_clock_mask.bits.g1 = color_clock_mask.bits.b1 = 1;
    color_clock_mask.bits.r2 = color_clock_mask.bits.g2 = color_clock_mask.bits.b2 = 1;
    SET_CLOCK(color_clock_mask.bits, 1);

    // Row mask
    row_mask.bits.row = 0x0f;

    // Clock
    SET_CLOCK(clock.bits, 1);

    // EO
    ENABLE_OUTPUT(output_enable.bits, 1);

    // Strobe
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

            for (b = COLOR_SHIFT + MAX_BITPLANES - pwm_bits; b < MAX_BITPLANES; ++b) {

                io_bits *row_data = lm_io_bits_value_at(bitplane, columns, d_row, 0, b);

                for (col = 0; col < columns; ++col) {
                    const io_bits out = *row_data++;
                    lm_gpio_set_masked_bits(out.raw, color_clock_mask.raw);
                    lm_gpio_set_bits(clock.raw);
                }

                lm_gpio_clear_bits(color_clock_mask.raw);

                lm_gpio_set_bits(strobe.raw);
                lm_gpio_clear_bits(strobe.raw);

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

    pthread_cond_init(&thread->halt_cond, NULL);
    pthread_mutex_init(&thread->halt_mutex, NULL);

    for (i = 0; i < MAX_BITPLANES; ++i) {
        thread->row_sleep_timings[i] = (1 << i) * base_time_nanos;
    }

    return thread;
}

void lm_thread_free(lmThread *thread) {
    lm_thread_stop(thread);
    lm_thread_wait(thread);
    pthread_cond_destroy(&thread->halt_cond);
    pthread_mutex_destroy(&thread->halt_mutex);
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


