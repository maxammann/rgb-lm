#include <stdlib.h>
#include "screen.h"
#include "../controller.h"

#include "menu_screens.h"

#define MENUES 3

#define NEW_MENU(i, text)  lm_fonts_populate_wstring(get_font_library(), menus[i], text, get_default_font());
#define NEW_LMENU(i, text, len) lm_fonts_populate_lstring(get_font_library(), menus[i], text, len, get_default_font());

lmString **menus;

int current_menu = 2;


void menu_screen_init() {
    int i;

    menus = malloc(MENUES * sizeof(lmString *));

    for (i = 0; i < MENUES; ++i) {
        menus[i] = lm_fonts_string_new();
    }

    NEW_MENU(0, L"⏰");
    NEW_MENU(1, L"☁");
    unsigned long speaker = 0x1F50A;
    NEW_LMENU(2, &speaker, 1);
}

void menu_screen(lmLedMatrix *matrix, long int elapsed) {
    lm_matrix_clear(matrix);

    rgb color = {0, 0, 255};

    lmString *menu = menus[current_menu];

    int width = lm_fonts_string_width(menu);
    int height = lm_fonts_string_height(menu);

//    lmMatrix m;
//    m.xx = 2;
//    m.xy = 0;
//    m.yx =0;
//    m.yy = 2;
//
//    lm_fonts_string_apply_transformation(menu, m);

    lm_fonts_render_string(matrix, menu, 16 - (width / 2), 16 - (height / 2), &color);
    lm_matrix_swap_buffers(matrix);
};

void register_menu_screens() {
    menu_screen_init();
    register_screen("menu", (screen_t) & menu_screen);
}
