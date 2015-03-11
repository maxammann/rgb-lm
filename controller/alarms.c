#include <glib.h>
#include <malloc.h>
#include "alarms.h"

struct Alarm_ {
    int enabled;
    uint32_t time;
    char *name;
};

GSList *alarms;

Alarm *new_alarm(char *name, uint32_t time, int enabled) {
    Alarm *alarm = malloc(sizeof(Alarm));
    alarm->name = name;
    alarm->time = time;
    alarm->enabled = enabled;
    return alarm;
}

char *get_name(Alarm *alarm) {
    if (alarm == NULL) {
        return "None";
    }
    return alarm->name;
}

uint32_t get_time(Alarm *alarm) {
    return alarm->time;
}

int get_enabled(Alarm *alarm) {
    return alarm->enabled;
}

void add_alarm(Alarm *alarm) {
    if (alarms == NULL) {
        alarms = g_slist_alloc();
    }

    alarms = g_slist_append(alarms, alarm);
}

void free_nodes(gpointer data) {
    free(data);
}

GSList *get_alarms() {
    return alarms;
}

void clear_alarms() {
    if (alarms != NULL) {
        g_slist_free_full(alarms, free_nodes);
        alarms = g_slist_alloc();
    }
}
