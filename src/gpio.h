#pragma once

#include "stdint.h"

#ifndef REV2
#ifndef REV1
#error "REV2 or REV1 not defined"
#endif
#endif

#ifdef REV1
#define SET_CLOCK(bits, c) bits.clock_rev1 = c
#endif
#ifdef REV2
#define SET_CLOCK(bits, c) bits.clock_rev2 = c
#endif


#ifdef REV1
#define ENABLE_OUTPUT(bits, o) bits.output_enable_rev1 = o
#endif
#ifdef REV2
#define ENABLE_OUTPUT(bits, o) bits.output_enable_rev2 = o
#endif


int lm_gpio_init();

uint32_t lm_gpio_init_output(uint32_t outputs);

void lm_gpio_set_bits(uint32_t value);

// Clear the bits that are '1' in the output. Leave the rest untouched.
void lm_gpio_clear_bits(uint32_t value);

// Write all the bits of "value" mentioned in "mask". Leave the rest untouched.
void lm_gpio_set_masked_bits(uint32_t value, uint32_t mask);
