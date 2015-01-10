#pragma once

#include <lm/lm.h>
#include "lm.pb-c.h"

void init_controller();

lmLedMatrix *get_matrix();

lmFontLibrary *get_font_library();

lmFont *get_default_font();

void free_controller();

void process_buffer(uint8_t *buffer, size_t size);
