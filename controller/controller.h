#pragma once

#include <lm/lm.h>
#include "lm.pb-c.h"

static const int UTF8_BUFERR_SIZE = 256;

void init_controller();

lmFontLibrary *get_font_library();

lmFont *get_default_font();

void free_controller();

void process_buffer(uint8_t *buffer, size_t size);
