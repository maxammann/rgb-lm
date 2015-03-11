#include <stdlib.h>
#include "screen.h"
#include "math.h"
#include "lm/ppm.h"

#include "menu.h"

#define MENUES 3

#define SPEED 25.0

#define NEW_MENU(i, path) ppm_load(path, menus[i]);

static image **menus;

static int next_menu = 0;
static enum Direction {
    LEFT, RIGHT, NOTHING
} next_direction = NOTHING;

static double next_x = 0;

static int current_menu = 0;

static double current_x;

void menu_next() {
    if (next_menu != current_menu) {
        return;
    }
    next_menu = (current_menu + 1) % MENUES;
    next_direction = LEFT;
}

void menu_previous() {
    if (next_menu != current_menu) {
        return;
    }

    if (current_menu == 0) {
        next_menu = MENUES - 1;
    } else {
        next_menu = (current_menu - 1) % MENUES;
    }

    next_direction = RIGHT;
}

void menu_screen_init() {
    int i;

    menus = malloc(MENUES * sizeof(lmString *));

    for (i = 0; i < MENUES; ++i) {
        menus[i] = ppm_new(); //todo possibly free?
    }

    NEW_MENU(0, "/root/projects/rgb-led-matrix/graphics/1.ppm");
    NEW_MENU(1, "/root/projects/rgb-led-matrix/graphics/2.ppm");
    NEW_MENU(2, "/root/projects/rgb-led-matrix/graphics/3.ppm");
}

static inline void move(double elapsed, int sign, double start) {
    if (next_x == 0) {
        next_x = start;
    }

    double delta = SPEED * elapsed;

    next_x = sign ? next_x + delta : next_x - delta;
    current_x = sign ? current_x + delta : current_x - delta;

    if (current_x > 32 || current_x < -32) {
        next_x = 0;
        current_menu = next_menu;
        next_direction = NOTHING;
        printf("finished\n");
    }
}

void menu_screen(lmLedMatrix *matrix, double elapsed) {
    lm_matrix_clear(matrix);

    image *current = menus[current_menu];

    if (next_direction == RIGHT) {
        image *next = menus[next_menu];
        int next_width = next->width;
        move(elapsed, 1, -next_width);
        ppm_render(matrix, (int16_t) round(next_x), 0, next);
    } else if (next_direction == LEFT) {
        image *next = menus[next_menu];
        int next_width = next->width;
        move(elapsed, 0, next_width);
        ppm_render(matrix, (int16_t) round(next_x), 0, next);
    } else {
        current_x = 0;
    }


//    printf("%d | %f | %f | %f\n", next_direction, current_x, next_x, elapsed);

    ppm_render(matrix, (int16_t) round(current_x), 0, current);

    lm_matrix_swap_buffers(matrix);
};

void register_menu_screens() {
    menu_screen_init();
    register_screen("menu", (screen_t) &menu_screen);
}
