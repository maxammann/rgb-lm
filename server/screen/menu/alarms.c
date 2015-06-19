#include <stdio.h>
#include "alarms.h"
#include "../../alarms.h"
#include "../../controller.h"

static rgb color = {0, 204, 204};

static uint32_t current_alarm = 0;

void alarms_menu_screen(lmLedMatrix *matrix, int16_t x, int16_t y, double elapsed, void *user_data) {

    uint32_t amount = get_alarms_size();

    if (current_alarm >= amount) {
        current_alarm = 0;
        return;
    }


    Alarm *alarms = get_alarms();
    char buffer[80];


    Alarm alarm = alarms[current_alarm];

    sprintf(buffer, "%02d:%02d",
            alarm.time / 3600,
            alarm.time % 3600 / 60
    );


    lm_fonts_print_string(get_font_library(), matrix, buffer, get_small_font(), x, y, &color);

    lm_fonts_print_string(get_font_library(), matrix, alarm.name, get_small_font(), x, y + 10, &color);
}

void next_alarm() {
    uint32_t amount = get_alarms_size();

    current_alarm = (current_alarm + 1) % amount;
}