#include <string.h>
#include "ppm.h"

#define PPMREADBUFLEN 256

image *ppm_new() {
    return malloc(sizeof(image));
}

void ppm_free(image *img) {
    free(img);
}

void ppm_load(char *file, image *img) {
    char buf[PPMREADBUFLEN], *t;

    unsigned int w, h, d;
    int r;
    FILE *pf = fopen(file, "r");

    if (pf == NULL) return;

    t = fgets(buf, PPMREADBUFLEN, pf);

    /* the code fails if the white space following "P6" is not '\n' */
    if ((t == NULL) || (strncmp(buf, "P6\n", 3) != 0)) return;
    do { /* Px formats can have # comments after first line */
        t = fgets(buf, PPMREADBUFLEN, pf);
        if (t == NULL) return;
    } while (strncmp(buf, "#", 1) == 0);
    r = sscanf(buf, "%u %u", &w, &h);
    if (r < 2) return;

    img->width = w;
    img->height = h;
    img->buf = malloc(sizeof(pixel) * w * h);

    r = fscanf(pf, "%u", &d);
    if ((r < 1) || (d != 255)) return;
    fseek(pf, 1, SEEK_CUR); /* skip one byte, should be whitespace */

    size_t rd = fread(img->buf, sizeof(pixel), w * h, pf);
    if (rd < w * h) {
        return;
    }
}

void ppm_render(lmLedMatrix *matrix, int16_t x_start, int16_t y_start, image *img) {
    uint16_t x, y;

    for (x = 0; x < img->width; ++x) {
        for (y = 0; y < img->height; ++y) {
            rgb rgb;
            pixel pixel = img->buf[y * img->width + x];
            rgb.r = pixel.r;
            rgb.g = pixel.g;
            rgb.b = pixel.b;

            lm_matrix_set_pixel(matrix, x_start + x, y_start + y, &rgb);
        }
    }
}
