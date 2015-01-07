#pragma once

#include <lm/lm.h>
#include "lm.pb-c.h"

void init_controller();

void free_controller();

inline lmFont *get_font(uint32_t key);

void fill(Lm__Fill *fill);

void set_pixel(Lm__SetPixel *set_pixel);

void create_font(Lm__CreateFont *create_font);

void destroy_font(Lm__DestroyFont *destroyFont);

void print_string(Lm__PrintString *print_string);

void process_buffer(uint8_t *buffer, size_t size);
