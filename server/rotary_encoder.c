#include <wiringPi.h>
#include <stdio.h>

#include "rotary_encoder.h"

//void updateEncoder() {
//    int rotary_a = digitalRead(encoder.pin_a);
//    int rotary_b = digitalRead(encoder.pin_b);
//
//    int seq = (rotary_a ^ rotary_b) | rotary_b << 1;
//
//    if (seq == encoder.last_seq) {
//        return;
//    }
//
//    int delta = (seq - encoder.last_seq) % 4;
//
//
//    if (delta == 3) {
//        delta = -1;
//    } else if (delta == 2) {
//        delta = encoder.delta;
//    }
//
//    printf("%d\n", delta);
//
//    switch (delta) {
//        case 1:
//            encoder.value++;
//            break;
//        case -1:
//            encoder.value--;
//            break;
//        default:
//            break;
//    }
//
//    encoder.last_seq = seq;
//    encoder.delta = delta;
//}

void updateEncoder() {

    int MSB = digitalRead(encoder.pin_a);
    int LSB = digitalRead(encoder.pin_b);

    int encoded = (MSB << 1) | LSB;
    int sum = (encoder.last_seq << 2) | encoded;

    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
        printf("+\n");
        encoder.value++;
    }
    if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
        printf("-\n");
        encoder.value--;
    }

    encoder.last_seq = encoded;
}

void setupencoder(int pin_a, int pin_b, int switch_pin, void (*switch_down)) {
    wiringPiSetup();

    encoder.pin_a = pin_a;
    encoder.pin_b = pin_b;
    encoder.value = 0;
    encoder.delta = 0;
    encoder.last_seq = 0;

    pinMode(pin_a, INPUT);
    pinMode(pin_b, INPUT);
    pullUpDnControl(pin_a, PUD_UP);
    pullUpDnControl(pin_b, PUD_UP);

    pinMode(switch_pin, INPUT);
    pullUpDnControl(switch_pin, PUD_UP);

    wiringPiISR(pin_a, INT_EDGE_BOTH, updateEncoder);
    wiringPiISR(pin_b, INT_EDGE_BOTH, updateEncoder);


    wiringPiISR(switch_pin, INT_EDGE_BOTH, switch_down);
}
