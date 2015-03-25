#include <glib.h>
#include <malloc.h>
#include <unistd.h>
#include <lm/thread.h>
#include <m3u.h>
#include "alarms.h"
#include "audio.h"
#include "controller.h"
#include "screen/screen.h"

struct Alarm_ {
    int enabled;
    uint32_t time;
    char *name;
};

static GSList *alarms;

static int running;

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

void check_alarm(gpointer data, gpointer user_data) {
    if (data == NULL) {
        return;
    }

    time_t t = time(NULL);
    struct tm *gtm = gmtime(&t);

    gtm->tm_hour = (gtm->tm_hour + 1) % 24;

    if (gtm->tm_hour > 10) {
        return;
    }

    Alarm *alarm = data;

    int wake = gtm->tm_hour * 60 * 60 + gtm->tm_min * 60 + gtm->tm_sec;

    if (alarm->time < wake) {
        set_current_screen(get_screen("menu"), NULL);
        lm_thread_unpause(get_thread());
        play("heaven.mp3");

        int amount;
        Title *titles = m3u_read("test.m3u", &amount);

        int i;
        for (i = 0; i < amount; ++i) {
            play(titles[0].title_dest);
        }

        m3u_free(titles, amount);
    }
}

void *watch(void *nil) {

    while (running) {
        g_slist_foreach(alarms, check_alarm, NULL);

        usleep(1000000);
    }

    return NULL;
}

void start_watch() {
    running = 1;
    pthread_t pthread;
    pthread_create(&pthread, NULL, watch, NULL);
}
