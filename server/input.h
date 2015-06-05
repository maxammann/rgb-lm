#pragma once

#include <stdint.h>

void input_setup();

int last_down_longer_than(unsigned long time);

int last_up_longer_than(unsigned long time);

void reset_last_down();

void reset_last_up();