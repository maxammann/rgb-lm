#pragma once

struct encoder_dev_ {
    int pin_a;
    int pin_b;
    int value;
    int last_seq;
    unsigned long last_rotatated;
};

typedef struct encoder_dev_ encoder_dev;

encoder_dev encoder;

void setupencoder(int pin_a, int pin_b, int switch_pin);

void input_setup();

int last_down_longer_than(unsigned long time);

int last_up_longer_than(unsigned long time);

int last_rotated_longer_than(unsigned long time);

void reset_last_down();

void reset_last_up();