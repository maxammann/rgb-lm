#pragma once

#include <stdint.h>
#include <lm/led-matrix.h>

void visualize_init(int samples);

void buffer_visualize(int16_t *data);

void screen_draw(lmLedMatrix *matrix, double elapsed);

void register_visualize_screen();
