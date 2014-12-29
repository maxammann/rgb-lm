#include "io_bits.h"
#include "gpio.h"

io_bits *lm_io_bits_value_at(io_bits *bitplane, int columns, int double_row, int column, int bit) {
    return &bitplane[double_row * (columns * MAX_BITPLANES)
            + bit * columns
            + column];
};

uint32_t lm_io_bits_new() {
    // Tell GPIO about all bits we intend to use.
    io_bits b;

    b.raw = 0;

    SET_CLOCK(b.bits, 1);
    ENABLE_OUTPUT(b.bits, 1);

    b.bits.strobe = 1;
    b.bits.r1 = b.bits.g1 = b.bits.b1 = 1;
    b.bits.r2 = b.bits.g2 = b.bits.b2 = 1;
    b.bits.row = 0x0f;

    return b.raw;
}
