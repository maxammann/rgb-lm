#include <glib.h>
#include <stdlib.h>
#include <lm.pb-c.h>
#include "controller.h"
#include "stdio.h"
#include "screen/screen.h"

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

//    lm_thread_pause(thread);
    lm_thread_start(thread);

    init_screens(matrix);


}

inline lmFont *get_font(uint32_t key) {
    return g_hash_table_lookup(fonts, &key);
}

static void fill(Lm__Fill *fill) {
    rgb rgb = TO_RGB(fill->rgb);
    lm_matrix_fill(matrix, &rgb);
}

static void set_pixel(Lm__SetPixel *set_pixel) {
    rgb rgb = TO_RGB(set_pixel->rgb);
    Lm__Position *position = set_pixel->pos;
    lm_matrix_set_pixel(matrix, (uint16_t) position->x, (uint16_t) position->y, &rgb);
}

static void create_font(Lm__CreateFont *create_font) {
    lmFont *font = lm_fonts_font_new(library, (char const *) create_font->font.data, create_font->size);

    uint32_t *key = malloc(sizeof(uint32_t));
    *key = create_font->id;

    g_hash_table_insert(fonts, key, font);
}

static void destroy_font(Lm__DestroyFont *destroyFont) {
    g_hash_table_remove(fonts, &destroyFont->id);
}


static void create_string(Lm__PopulateString *populateString) {
    lmString *string = lm_fonts_string_new();
    lm_fonts_populate_string(library, string, populateString->string.data, get_font(populateString->font));


    uint32_t *key = malloc(sizeof(uint32_t));
    *key = populateString->id;

    g_hash_table_insert(strings, key, string);
}

static void destroy_string(Lm__DestroyString *destroy_string) {
    g_hash_table_remove(strings, &destroy_string->id);
}

static void render_string(Lm__RenderString *render_string) {
    lmString *string = g_hash_table_lookup(strings, &render_string->id);

    rgb rgb = TO_RGB(render_string->rgb);
    Lm__Position *position = render_string->pos;
    lm_fonts_render_string(matrix, string, position->x, position->y, &rgb);
}

static void print_string(Lm__PrintString *print_string) {
    uint32_t font = print_string->font;
    Lm__Position *position = print_string->pos;
    rgb rgb = TO_RGB(print_string->rgb);
    char *text = (char *) print_string->string.data;

    lm_fonts_print_string(library, matrix, text, get_font(font), position->x, position->y, &rgb);
}

static void set_screen(Lm__SetScreen *set_screen) {
    char *name = set_screen->name;
    screen_t screen = get_screen(name);

    if (screen == NULL) {
        printf("Screen does not exist! %s\n", name);
        return;
    }
    set_current_screen(matrix, screen);
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
        case LM__REQUEST__TYPE__DESTROYSTRING:
            destroy_string(request->destroystring);
            break;
        case LM__REQUEST__TYPE__POPULATESTRING:
            create_string(request->populatestring);
            break;
        case LM__REQUEST__TYPE__RENDERSTRING:
            render_string(request->renderstring);
            break;
        case LM__REQUEST__TYPE__SWAPBUFFERS:
            lm_matrix_swap_buffers(matrix);
            break;
        case LM__REQUEST__TYPE__PAUSE:
            lm_thread_pause(thread);
            break;
        case LM__REQUEST__TYPE__UNPAUSE:
            lm_thread_unpause(thread);
            break;
        case LM__REQUEST__TYPE__SETSCREEN:
            set_screen(request->setscreen);
            break;
        default:
            printf("Unknown type: %d\n", request->type);

    }

    printf("Type: %d\n", request->type);
}

void free_controller() {
    lm_matrix_free(matrix);
    lm_thread_free(thread);
    lm_fonts_free(library);
}
