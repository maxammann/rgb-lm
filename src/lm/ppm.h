#include <stdio.h>
#include "led-matrix.h"
#include <malloc.h>
#include "led-matrix.h"


struct pixel_ {
    uint8_t r, g, b;
};

typedef struct pixel_ pixel;

struct image_ {
    pixel *buf;
    int width, height;
};

typedef struct image_ image_t;

image_t *ppm_new();

void ppm_free(image_t *img);

void ppm_load(char *file, image_t *img);

void ppm_render(lmLedMatrix *matrix, int16_t x, int16_t y, image_t *img);

