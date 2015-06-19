#pragma once

#include <stdint.h>
#include "../../../src/lm/led-matrix.h"

void digital_clock_menu_screen(lmLedMatrix *matrix, int16_t x, int16_t y, double elapsed, void *user_data);