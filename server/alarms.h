#pragma once

#include <stdint.h>

struct Alarm_ {
    int enabled;
    uint32_t time;
    char *name;

    int already_woke;
};

typedef struct Alarm_ Alarm;

void set_alarms(Alarm *alarms, size_t alarms_size);

void clear_alarms();

void write_alarms(char *path);

void read_alarms(char *path);

uint32_t get_alarms_size();

Alarm *get_alarms();
