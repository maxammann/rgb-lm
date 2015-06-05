#include <stdlib.h>
#include <string.h>
#include "stdio.h"

#include "controller.h"
#include "alarms.h"

#include "screen/menu.h"
#include "screen/screen.h"

static lmLedMatrix *matrix;
static lmThread *thread;
static lmFontLibrary *library;
static lmFont *default_font;

void init_controller() {
    lm_gpio_init();
    lm_gpio_init_output(lm_io_bits_new());

    matrix = lm_matrix_new(32, 32, 6);
    thread = lm_thread_new(matrix, DEFAULT_BASE_TIME_NANOS);

    library = lm_fonts_init();

//    default_font = lm_fonts_font_new(library, "alarm-clock/fonts/Symbola.ttf", 22);
//    default_font = lm_fonts_font_new(library, "alarm-clock/fonts/NotoSans-Regular.ttf", 20);
    default_font = lm_fonts_font_new(library, "alarm-clock/fonts/alterebro-pixel-font.ttf", 32);

    lm_thread_start(thread);
    lm_thread_pause(thread);

    screens_start(matrix);
}

static inline void proto_set_screen(Lm__SetScreen *set_screen) {
    char *name = set_screen->name;
    if (name[0] == '\0') {
        set_current_screen(NULL, NULL);
    }

    screen_t screen = get_screen(name);

    if (screen == NULL) {
        printf("Screen does not exist! %s\n", name);
        return;
    }
    set_current_screen(screen, NULL);
}

void proto_set_alarms(Lm__Alarms *alarms) {
    int i;

    Alarm *out_alarms = malloc(alarms->n_alarms * sizeof(Alarm));

    clear_alarms();

    for (i = 0; i < alarms->n_alarms; i++) {
        Lm__Alarm *proto_alarm = alarms->alarms[i];

        char *name = strdup(proto_alarm->name);

        Alarm alarm;

        alarm.name = name;
        alarm.time = proto_alarm->time;
        alarm.enabled = proto_alarm->enabled;
        out_alarms[i] = alarm;
        printf("Registered alarm %s\n", name);
    }

    set_alarms(out_alarms, alarms->n_alarms);
}

void process_buffer(uint8_t *buffer, size_t size) {
    Lm__Request *request = lm__request__unpack(NULL, size, (uint8_t const *) buffer);

    switch (request->type) {
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
            proto_set_screen(request->setscreen);
            break;
        case LM__REQUEST__TYPE__CLEAR:
            lm_matrix_clear(matrix);
            break;
        case LM__REQUEST__TYPE__ALARM_REQUST:
            proto_set_alarms(request->alarm_request->alarms);
            break;
        case LM__REQUEST__TYPE__MENU_NEXT:
            menu_next();
            break;
        case LM__REQUEST__TYPE__MENU_PREVIOUS:
            menu_previous();
            break;
        default:
            printf("Unknown type: %d\n", request->type);

    }

    printf("Received response type: %d\n", request->type);
    lm__request__free_unpacked(request, NULL);
}


lmLedMatrix *get_matrix() {
    return matrix;
}

lmThread *get_thread() {
    return thread;
}

lmFontLibrary *get_font_library() {
    return library;
}

lmFont *get_default_font() {
    return default_font;
}

void free_controller() {
    lm_matrix_free(matrix);
    lm_thread_free(thread);
    lm_fonts_font_free(library, default_font);
    lm_fonts_free(library);
}
