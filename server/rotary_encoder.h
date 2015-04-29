struct encoder_dev_ {
    int pin_a;
    int pin_b;
    int value;
    int delta;
    int last_seq;
};

typedef struct encoder_dev_ encoder_dev;

encoder_dev encoder;

void setupencoder(int pin_a, int pin_b, int switch_pin, void (*switch_down));
