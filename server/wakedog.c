#include <stdlib.h>
#include <m3u.h>
#include <stdio.h>
#include <unistd.h>
#include "alarms.h"
#include "screen/screen.h"
#include "controller.h"
#include "audio.h"
#include "wakedog.h"

static int running;
static Alarm *currently_running;

//stop playback
static int is_playback_stopped_ = 0;

int is_playback_stopped() {
    return is_playback_stopped_;
}

void *wakeup(void *pVoid) {
    char *wakup_audio = getenv("WAKEUP_AUDIO");
    char *news_playlist = getenv("NEWS_PLAYLIST");

    set_current_screen(get_screen("menu"), NULL);
    lm_thread_unpause(get_thread());
    play(wakup_audio, 30, is_playback_stopped);

    size_t amount;
    Title *titles = m3u_read(news_playlist, &amount);

    int i;
    for (i = 0; i < amount; ++i) {
        play(titles[i].title_dest, 0, is_playback_stopped);
    }

    m3u_free(titles, amount);

    currently_running = 0;

    return 0;
}

void check_alarm(Alarm *alarm) {
    if (alarm == NULL) {
        return;
    }

    if (currently_running != NULL) {
        return;
    }

    time_t t = time(NULL);
    struct tm *gtm = gmtime(&t);

    gtm->tm_hour = (gtm->tm_hour + 1) % 24;

    int time = gtm->tm_hour * 60 * 60 + gtm->tm_min * 60 + gtm->tm_sec;

    int32_t delta = time - alarm->time;

    if (delta > 0 && delta < 2) {
        currently_running = alarm;
        pthread_t audio;
        pthread_create(&audio, NULL, wakeup, NULL);
        printf("Waking you up! Motherfucker, it's %d:%d!\n", gtm->tm_hour, gtm->tm_min);
    }
}

void *watch(void *nil) {
    int i;
    uint32_t size = get_alarms_size();
    Alarm *alarms = get_alarms();

    while (running) {

        for (i = 0; i < size; ++i) {
            check_alarm(&alarms[i]);
        }

        usleep(SLEEP);
    }

    return NULL;
}

void start_dog() {
    running = 1;
    pthread_t pthread;
    pthread_create(&pthread, NULL, watch, NULL);
}
