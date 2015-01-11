#pragma once

#include "led-matrix.h"

typedef unsigned int font_size_t;

typedef struct lmFont_ lmFont;

typedef struct lmString_ lmString;

typedef struct lmMatrix_ lmMatrix;

typedef struct lmFontLibrary_ lmFontLibrary;

struct lmMatrix_ {
    int xx, xy;
    int yx, yy;
};

lmFontLibrary *lm_fonts_init();

void lm_fonts_free(lmFontLibrary *);

lmFont *lm_fonts_font_new(lmFontLibrary *library, const char *font, font_size_t size);

void lm_fonts_font_free(lmFontLibrary *library, lmFont *font);


lmString *lm_fonts_string_new();

int lm_fonts_string_width(lmString *string);

int lm_fonts_string_height(lmString *string);

void lm_fonts_string_apply_transformation(lmString *string, lmMatrix matrix);

void lm_fonts_string_free(lmString *string);

void lm_fonts_populate_string(lmFontLibrary *library, lmString *string, const char *text, lmFont *font);

void lm_fonts_populate_wstring(lmFontLibrary *library, lmString *string, const wchar_t *text, lmFont *font);

void lm_fonts_render_string(lmLedMatrix *matrix, lmString *string,
        uint16_t x, uint16_t y,
        rgb *rgb);

void lm_fonts_print_string(lmFontLibrary *library, lmLedMatrix *matrix, const char *text, lmFont *font,
        uint16_t x, uint16_t y,
        rgb *rgb);

void lm_fonts_print_wstring(lmFontLibrary *library, lmLedMatrix *matrix, const wchar_t *text, lmFont *font,
        uint16_t x, uint16_t y,
        rgb *rgb);

void lm_fonts_print_lstring(lmFontLibrary *library, lmLedMatrix *matrix, unsigned long *text, int len, lmFont *font,
        uint16_t x, uint16_t y,
        rgb *rgb);

void lm_fonts_populate_lstring(lmFontLibrary *library, lmString *string, unsigned long *text, int len, lmFont *font);
