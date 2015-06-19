#pragma once

typedef void (*menu_screen_t)(lmLedMatrix *matrix, int16_t x, int16_t y, double elapsed, void *user_data);

void menu_next();

void menu_previous();

void menu_screen(lmLedMatrix *matrix, double elapsed);

int get_current_menu();

void register_menu_screens();

