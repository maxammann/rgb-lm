#include <wiringPi.h>

#include "rotary_encoder.h"

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
}

void setupencoder(int pin_a, int pin_b, int switch_pin, void (*switch_button)) {
    wiringPiSetup();

    encoder.pin_a = pin_a;
    encoder.pin_b = pin_b;
    encoder.value = 0;
    encoder.last_seq = 0;

    pinMode(pin_a, INPUT);
    pinMode(pin_b, INPUT);
    pullUpDnControl(pin_a, PUD_UP);
    pullUpDnControl(pin_b, PUD_UP);

    pinMode(switch_pin, INPUT);
    pullUpDnControl(switch_pin, PUD_UP);

    wiringPiISR(pin_a, INT_EDGE_BOTH, updateEncoder);
    wiringPiISR(pin_b, INT_EDGE_BOTH, updateEncoder);

    wiringPiISR(switch_pin, INT_EDGE_BOTH, switch_button);
}
