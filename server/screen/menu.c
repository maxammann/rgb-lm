#include <stdlib.h>
#include "screen.h"
#include "lm/ppm.h"
#include "math.h"

#include "menu.h"
#include "../rotary_encoder.h"

#include "menu/clock.h"
#include "menu/mesmerizing.h"
#include "menu/alarms.h"

#define MENUES 3

#define ENCODER_EDGE 15
#define SPEED 25.0

#define WIDTH 32

static enum Direction {
    NOTHING = 0, LEFT = -1, RIGHT = 1,
} next_direction = NOTHING;


typedef struct menu_ menu_t;

struct menu_ {
    menu_screen_t screen;
    char *name;
    void *user_data;
};

//void ppm_menu_screen(lmLedMatrix *matrix, int16_t x, int16_t y, double elapsed, void *user_data) {
//    ppm_render(matrix, x, y, user_data);
//}


static menu_t menus[MENUES];
static int current_menu = 0;
static int next_menu = 0;


void menu_screen_init() {
    menu_t menu;

    menu.name = "clock";
    menu.screen = digital_clock_menu_screen;
    menus[0] = menu;

    menu.name = "mesmerizing";
    menu.screen = mesmerizing_menu_screen;
    menus[1] = menu;

    menu.name = "alarms";
    menu.screen = alarms_menu_screen;
    menus[2] = menu;
}

void menu_next() {
    if (next_menu != current_menu) {
        return;
    }

    next_menu = (current_menu + 1) % MENUES;
    next_direction = LEFT;
#ifdef DEBUG
    printf("Left\n");
#endif
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
#ifdef DEBUG
    printf("Right\n");
#endif
}

static double next_x = 0;
static double current_x = 0;

static inline void move_menu(double elapsed, int sign) {
    if (next_x == 0) {
        next_x = -sign * WIDTH; //set next outside of screen
    }

    double delta = SPEED * elapsed;

    next_x += sign * delta;
    current_x += sign * delta;

    if (current_x > WIDTH || current_x < -WIDTH) {
        next_x = 0;
        current_menu = next_menu;
        next_direction = NOTHING; // finished for this menu
#ifdef DEBUG
        printf("Finished transition\n");
#endif
    }
}

void menu_screen(lmLedMatrix *matrix, double elapsed) {
    if (encoder.value / ENCODER_EDGE > 1) {
        menu_next();
#ifdef DEBUG
        printf("next ->\n");
#endif
        encoder.value = 0;
    } else if (encoder.value / ENCODER_EDGE < -1) {
        menu_previous();
#ifdef DEBUG
        printf("previous ->\n");
#endif
        encoder.value = 0;
    }

    lm_matrix_clear(matrix);

    menu_t current = menus[current_menu];

    if (next_direction == NOTHING) {
        current_x = 0; // Nothing to do
    } else {
        menu_t next = menus[next_menu];

        move_menu(elapsed, next_direction);
        next.screen(matrix, (int16_t) round(next_x), 0, elapsed, next.user_data);
    }

    current.screen(matrix, (int16_t) round(current_x), 0, elapsed, current.user_data);

    lm_matrix_swap_buffers(matrix);
}

int get_current_menu() {
    return current_menu;
}


void register_menu_screens() {
    menu_screen_init();
    register_screen("menu", (screen_t) &menu_screen);
}
