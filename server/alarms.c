#include <malloc.h>
#include <unistd.h>
#include <lm/thread.h>
#include <stdlib.h>
#include <string.h>
#include "alarms.h"

static Alarm *alarms;
static uint32_t alarms_size;

void set_alarms(Alarm *alarms_, size_t alarms_size_) {
    alarms = alarms_;
    alarms_size = (uint32_t) alarms_size_;

    write_alarms("test.alarms");
}

void clear_alarms() {
    int i;

    for (i = 0; i < alarms_size; ++i) {
        Alarm alarm = alarms[i];

        free(alarm.name);
    }

    free(alarms);
    alarms = 0;
}

void write_alarms(char *path) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        return;
    }

    fwrite(&alarms_size, sizeof(uint32_t), 1, file);

    int i;

    for (i = 0; i < alarms_size; ++i) {
        Alarm alarm = alarms[i];

        fwrite(&alarm.enabled, sizeof(int), 1, file);
        fwrite(&alarm.time, sizeof(uint32_t), 1, file);

        uint32_t name_length = (uint32_t) (strlen(alarm.name) + 1);

        fwrite(&name_length, sizeof(int32_t), 1, file);
        fwrite(alarm.name, sizeof(char), name_length, file);
    }


    fclose(file);
}

void read_alarms(char *path) {
    if (!access(path, F_OK)) {
        return;
    }

    clear_alarms();

    FILE *file = fopen(path, "r");
    if (file != NULL) {
        fread(&alarms_size, sizeof(uint32_t), 1, file);
        alarms = malloc(sizeof(Alarm) * alarms_size);
        int i;

        for (i = 0; i < alarms_size; ++i) {
            Alarm alarm;

            fread(&alarm.enabled, sizeof(int), 1, file);
            fread(&alarm.time, sizeof(uint32_t), 1, file);

            uint32_t name_length;
            fread(&name_length, sizeof(uint32_t), 1, file);

            alarm.name = malloc(sizeof(char) * name_length);

            fread(alarm.name, sizeof(char), name_length, file);

            alarms[i] = alarm;
        }

        fclose(file);
    }
}

uint32_t get_alarms_size() {
    return alarms_size;
}

Alarm *get_alarms() {
    return alarms;
}
