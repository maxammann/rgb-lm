#pragma once

#include <lm/lm.h>

void alarms_menu_screen(lmLedMatrix *matrix, int16_t x, int16_t y, double elapsed, void *user_data);

void next_alarm();