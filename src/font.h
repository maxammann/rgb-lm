#pragma once

#include "led-matrix.h"

typedef unsigned int font_size_t;

typedef struct lmFont_ lmFont;

typedef struct lmString_ lmString;


lmFont *lm_matrix_font_create(char *font, font_size_t size);

void lm_matrix_font_destroy(lmFont *font);


void lm_matrix_create_string(lmString *string, const char *text, lmFont *font);

void lm_matrix_create_wstring(lmString *string, const wchar_t *text, lmFont *font);

void lm_matrix_render_string(lmLedMatrix *matrix, lmString *string,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue);


void lm_matrix_print_string(lmLedMatrix *matrix, const char *text, lmFont *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue);

void lm_matrix_print_wstring(lmLedMatrix *matrix, const wchar_t *text, lmFont *font,
        uint16_t x, uint16_t y,
        uint8_t red, uint8_t green, uint8_t blue);
