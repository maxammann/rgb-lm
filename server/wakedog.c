#include <stdlib.h>
#include <m3u.h>
#include <stdio.h>
#include <unistd.h>
#include <uthash.h>
#include "alarms.h"
#include "screen/screen.h"
#include "audio.h"
#include "wakedog.h"

static int running;


//stop playback
static int is_playback_stopped_ = 0;

int is_playback_stopped() {
    return is_playback_stopped_;
}

static void *wakeup(void *pVoid) {
    size_t amount;
    Title *titles;

    char *wakeup_playlist = getenv("WAKEUP_PLAYLIST");

    if (wakeup_playlist != NULL) {
        titles = m3u_read(wakeup_playlist, &amount);

        play(titles[rand() % amount].title_dest, 0, is_playback_stopped);

        m3u_free(titles, amount);
    } else {
        printf("No audio found!\n");
        return 0;
    }

    char *news_playlist = getenv("NEWS_PLAYLIST");

    if (news_playlist != NULL) {
        titles = m3u_read(news_playlist, &amount);

        int i;
        for (i = 0; i < amount; ++i) {
            play(titles[i].title_dest, 0, is_playback_stopped);
        }

        m3u_free(titles, amount);
    } else {
        printf("No news found!\n");
        return 0;
    }

    return 0;
}

int should_be_woke_(Alarm *alarm, struct tm *now) {
    return now->tm_hour * 60 * 60 + now->tm_min * 60 + now->tm_sec > alarm->time;
}

int should_be_woke(Alarm *alarm) {
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    return should_be_woke_(alarm, now);
}

static void check_alarm(Alarm *alarm, struct tm *now) {
    if (alarm == NULL) {
        return;
    }

    if (alarm->already_woke == 1) {
        return;
    }


    if (should_be_woke_(alarm, now)) {
        alarm->already_woke = 1;

        pthread_t audio;
        pthread_create(&audio, NULL, wakeup, NULL);
        printf("Waking you up! Motherfucker, it's %d:%d!\n", now->tm_hour, now->tm_min);
    }
}

static void *watch(void *nil) {
    int i;
    int last_hour = 0;


    while (running) {
        time_t t = time(NULL);
        struct tm *now = localtime(&t);

        int reset = (last_hour == 23 && now->tm_hour == 0);

        uint32_t size = get_alarms_size();
        Alarm *alarms = get_alarms();

        for (i = 0; i < size; ++i) {
            Alarm *alarm = alarms + i;

            if (reset) {
                alarm->already_woke = 0;
            }

            check_alarm(alarm, now);
        }

        last_hour = now->tm_hour;

        usleep(SLEEP);
    }

    return NULL;
}

void start_dog() {
    running = 1;
    pthread_t pthread;
    pthread_create(&pthread, NULL, watch, NULL);
}
