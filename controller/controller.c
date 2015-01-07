#include <glib.h>
#include <stdlib.h>
#include "controller.h"
#include "stdio.h"

#define TO_RGB(net_rgb) {(uint8_t) net_rgb->r, (uint8_t) net_rgb->g, (uint8_t) net_rgb->b};

lmLedMatrix *matrix;
lmThread *thread;
lmFontLibrary *library;
GHashTable *fonts, *strings;

void init_controller() {
    lm_gpio_init();
    lm_gpio_init_output(lm_io_bits_new());

    matrix = lm_matrix_new(32, 32, 11);
    thread = lm_thread_new(matrix, DEFAULT_BASE_TIME_NANOS);

    library = lm_fonts_init();
    fonts = g_hash_table_new(g_int_hash, g_int_equal);
    strings = g_hash_table_new(g_int_hash, g_int_equal);

    lm_thread_pause(thread);
    lm_thread_start(thread);
}

inline lmFont *get_font(uint32_t key) {
    return g_hash_table_lookup(fonts, &key);
}

void fill(Lm__Fill *fill) {
    rgb rgb = TO_RGB(fill->rgb);
    lm_matrix_fill(matrix, &rgb);
    lm_matrix_swap_buffers(matrix);
}

void set_pixel(Lm__SetPixel *set_pixel) {
    rgb rgb = TO_RGB(set_pixel->rgb);
    Lm__Position *position = set_pixel->pos;
    lm_matrix_set_pixel(matrix, (uint16_t) position->x, (uint16_t) position->y, &rgb);
}

void create_font(Lm__CreateFont *create_font) {
    lmFont *font = lm_fonts_font_new(library, (char const *) create_font->font.data, create_font->size);

    uint32_t *key = malloc(sizeof(uint32_t));
    *key = create_font->id;

    g_hash_table_insert(fonts, key, font);
}

void destroy_font(Lm__DestroyFont *destroyFont) {
    uint32_t *key = malloc(sizeof(uint32_t));
    *key = destroyFont->id;

    g_hash_table_remove(fonts, key);
}

void print_string(Lm__PrintString *print_string) {
    uint32_t font = print_string->font;
    Lm__Position *position = print_string->pos;
    rgb rgb = TO_RGB(print_string->rgb);
    char *text = (char *) print_string->string.data;

    lm_fonts_print_string(library, matrix, text, get_font(font), position->x, position->y, &rgb);
}

void process_buffer(uint8_t *buffer, size_t size) {
    Lm__Request *request = lm__request__unpack(NULL, size, (uint8_t const *) buffer);

    switch (request->type) {
        case LM__REQUEST__TYPE__SETPIXEL:
            set_pixel(request->setpixel);
            break;
        case LM__REQUEST__TYPE__FILL:
            fill(request->fill);
            break;
        case LM__REQUEST__TYPE__CREATEFONT:
            create_font(request->createfont);
            break;
        case LM__REQUEST__TYPE__DESTROYFONT:
            destroy_font(request->destroyfont);
            break;
        case LM__REQUEST__TYPE__PRINTSTRING:
            print_string(request->printstring);
            break;
        case LM__REQUEST__TYPE__CREATESTRING:
            break;
        case LM__REQUEST__TYPE__DESTROYSTRING:
            break;
        case LM__REQUEST__TYPE__POPULATESTRING:
            break;
        case LM__REQUEST__TYPE__RENDERSTRING:
            break;
        case LM__REQUEST__TYPE__SWAPBUFFERS:
            lm_matrix_swap_buffers(matrix);
            break;
    }

    printf("Type: %d\n", request->type);
}

void free_controller() {
    lm_matrix_free(matrix);
    lm_thread_free(thread);
    lm_fonts_free(library);
}
