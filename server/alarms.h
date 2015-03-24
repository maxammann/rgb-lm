#pragma once

#include <stdint.h>
#include "glib.h"

typedef struct Alarm_ Alarm;

Alarm *new_alarm(char* name, uint32_t time, int enabled);

void add_alarm(Alarm *alarm);

void clear_alarms();

GSList *get_alarms();

char *get_name(Alarm *alarm);

uint32_t get_time(Alarm *alarm);

int get_enabled(Alarm *alarm);

void start_watch();
