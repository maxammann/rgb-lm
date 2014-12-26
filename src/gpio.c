#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>


#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define BLOCK_SIZE (4*1024)

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))

volatile unsigned *gpio;

static const uint32_t gpios = (
#ifdef REV1
        (1 << 0) | (1 << 1) |
#endif
#ifdef REV2
        (1 << 2) | (1 << 3) |
#endif
                (1 << 4) | (1 << 7) | (1 << 8) | (1 << 9) |
                (1 << 10) | (1 << 11) | (1 << 14) | (1 << 15) | (1 << 17) | (1 << 18) |
                (1 << 22) | (1 << 23) | (1 << 24) | (1 << 25) | (1 << 27));


void lm_gpio_init() {
    if (gpio != NULL) {
        return;
    }

    int mem_fd;

    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        printf("can't open /dev/mem \n");
        exit(-1);
    }

    void *gpio_map = mmap(
            NULL,                          //Any adddress in our space will do
            BLOCK_SIZE,                    //Map length
            PROT_READ | PROT_WRITE,        // Enable reading & writting to mapped memory
            MAP_SHARED,                    //Shared with other processes
            mem_fd,                        //File to map
            GPIO_BASE                      //Offset to GPIO peripheral
    );

    close(mem_fd);

    if (gpio_map == MAP_FAILED) {
        printf("mmap error %ld\n", (long) gpio_map);
        exit(-1);
    }

    // Always use volatile pointer!
    gpio = (volatile unsigned *) gpio_map;
}

uint32_t lm_gpio_init_output(uint32_t outputs) {
    if (gpio == NULL) {
        return ~outputs;
    }

    uint32_t b;

    outputs &= gpios;   // Sanitize input.
    uint32_t output_bits_ = outputs;

    for (b = 0; b < 27; ++b) {
        if (outputs & (1 << b)) {
            INP_GPIO(b);   // for writing, we first need to set as input.
            OUT_GPIO(b);
        }
    }
    return output_bits_;
}

void lm_gpio_set_bits(uint32_t value) {
    gpio[0x1C / sizeof(uint32_t)] = value;
}

void lm_gpio_clear_bits(uint32_t value) {
    gpio[0x28 / sizeof(uint32_t)] = value;
}

// Write all the bits of "value" mentioned in "mask". Leave the rest untouched.
inline void lm_gpio_set_masked_bits(uint32_t value, uint32_t mask) {
    // Writing a word is two operations. The IO is actually pretty slow, so
    // this should probably  be unnoticable.
    lm_gpio_clear_bits(~value & mask);
    lm_gpio_set_bits(value & mask);
}
