#include <malloc.h>
#include <lm/thread.h>
#include <stdlib.h>
#include <string.h>
#include "alarms.h"
#include "wakedog.h"

Alarm *alarms;
uint32_t alarms_size;

void set_alarms(Alarm *alarms_, size_t alarms_size_) {
    size_t i;

    for (i = 0; i < alarms_size_; ++i) {
        alarms_[i].already_woke = should_be_woke(&alarms_[i]);
    }

    alarms = alarms_;
    alarms_size = (uint32_t) alarms_size_;

    write_alarms("test.alarms");
}

void clear_alarms() {
    size_t i;

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
    clear_alarms();

    FILE *file = fopen(path, "r");
    if (file != NULL) {
        size_t read_size;
        fread(&read_size, sizeof(uint32_t), 1, file);
        Alarm *read = malloc(sizeof(Alarm) * read_size);
        int i;

        for (i = 0; i < read_size; ++i) {
            Alarm alarm;

            fread(&alarm.enabled, sizeof(int), 1, file);
            fread(&alarm.time, sizeof(uint32_t), 1, file);

            uint32_t name_length;
            fread(&name_length, sizeof(uint32_t), 1, file);

            alarm.name = malloc(sizeof(char) * name_length);

            fread(alarm.name, sizeof(char), name_length, file);

            read[i] = alarm;
        }

        fclose(file);

        set_alarms(read, read_size);
    }
}

uint32_t get_alarms_size() {
    return alarms_size;
}

Alarm *get_alarms() {
    return alarms;
}
