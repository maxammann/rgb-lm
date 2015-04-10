#pragma once

#include <stdint.h>
#include <lm/led-matrix.h>

void buffer_visualize(uint16_t *data, int n);

void screen_draw(lmLedMatrix *matrix, double elapsed); 

void register_visualize_screen();
