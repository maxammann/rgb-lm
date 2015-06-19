#include <wiringPi.h>

#include "rotary_encoder.h"
#include "time_util.h"
#include "screen/screen.h"
#include "controller.h"
#include "wakedog.h"
#include "screen/menu/alarms.h"
#include "screen/menu.h"

static unsigned long last_down = 0;
static unsigned long last_up = 0;

static void button() {
    int down;

    down = !digitalRead(15);

    unsigned long now = current_time();

    if (down) {
        last_down = now;
        last_up = 0;

        if (get_current_screen() == NULL) {
            set_current_screen(get_screen("menu"), NULL);
            lm_thread_unpause(get_thread());
        }

        skip_current_playback();
#ifdef DEBUG
        printf("Button down\n");
#endif
    } else {
        last_down = 0;
        last_up = now;

        if (get_current_menu() == 2) {
            next_alarm();
        }

#ifdef DEBUG
        printf("Button up\n");
#endif
    }
}

int last_down_longer_than(unsigned long time) {
    unsigned long now = current_time();
    return last_down != 0 && last_down < now - time;
}

int last_up_longer_than(unsigned long time) {
    unsigned long now = current_time();
    return last_up != 0 && last_up < now - time;
}

int last_rotated_longer_than(unsigned long time) {
    unsigned long now = current_time();
    return encoder.last_rotatated != 0 && encoder.last_rotatated < now - time;
}

void reset_last_down() {
    last_down = 0;
}

void reset_last_up() {
    last_up = 0;
}

void updateEncoder() {

    int MSB = digitalRead(encoder.pin_a);
    int LSB = digitalRead(encoder.pin_b);

    int encoded = (MSB << 1) | LSB;
    int sum = (encoder.last_seq << 2) | encoded;

    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
        encoder.value++;
    }
    if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
        encoder.value--;
    }

    encoder.last_seq = encoded;
    encoder.last_rotatated = current_time();
}

void setupencoder(int pin_a, int pin_b, int switch_pin) {
    wiringPiSetup();

    encoder.pin_a = pin_a;
    encoder.pin_b = pin_b;
    encoder.value = 0;
    encoder.last_seq = 0;
    encoder.last_rotatated = 0;

    pinMode(pin_a, INPUT);
    pinMode(pin_b, INPUT);
    pullUpDnControl(pin_a, PUD_UP);
    pullUpDnControl(pin_b, PUD_UP);

    pinMode(switch_pin, INPUT);
    pullUpDnControl(switch_pin, PUD_UP);

    wiringPiISR(pin_a, INT_EDGE_BOTH, updateEncoder);
    wiringPiISR(pin_b, INT_EDGE_BOTH, updateEncoder);

    wiringPiISR(switch_pin, INT_EDGE_BOTH, button);
}
