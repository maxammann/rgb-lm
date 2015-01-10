#pragma once

#include <lm/lm.h>
#include "lm.pb-c.h"

void init_controller();

void free_controller();

void process_buffer(uint8_t *buffer, size_t size);
