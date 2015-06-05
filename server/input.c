#include "input.h"
#include <wiringPi.h>
#include <time.h>
#include <stdio.h>
#include "rotary_encoder.h"
#include "math.h"
#include "screen/screen.h"
#include "controller.h"
#include "wakedog.h"

static unsigned long last_down = 0;
static unsigned long last_up = 0;


static unsigned long current_time() {
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    return spec.tv_sec * 1000 + (long) round(spec.tv_nsec / 1.0e6);
}

static void button() {
    int down;
    
    down = !digitalRead(15);

    unsigned long now = current_time();

    if (down) {
        last_down = now;
        last_up = 0;

        if (get_current_screen() == NULL) {
            set_current_screen(get_screen("menu_t"), NULL);
            lm_thread_unpause(get_thread());
        }

        skip_current_playback();
        printf("Button down\n");
    } else {
        last_down = 0;
        last_up = now;
        printf("Button up\n");
    }
}

void input_setup() {
    printf("Initialising encoder\n");
    setupencoder(9, 16, 15, button);
}

int last_down_longer_than(unsigned long time) {
    unsigned long now = current_time();
    return last_down != 0 && last_down < now - time;
}

int last_up_longer_than(unsigned long time) {
    unsigned long now = current_time();
    return last_up != 0 && last_up < now - time;
}

void reset_last_down() {
    last_down = 0;
}

void reset_last_up() {
    last_up = 0;
}