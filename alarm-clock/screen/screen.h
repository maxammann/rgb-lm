#pragma once

#include <lm/lm.h>

typedef void (*start_screen)(lmLedMatrix *matrix, double, void *user_data);

typedef start_screen screen_t;


void screens_start(lmLedMatrix *matrix);

void screens_stop();

screen_t get_current_screen();

screen_t set_current_screen(start_screen screen, void *user_data);

screen_t get_screen(const char *name);

void register_screen(const char *name, screen_t screen);

void close_screens();
