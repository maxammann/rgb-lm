#include <stdlib.h>
#include <string.h>
#include "uthash.h"
#include "screen.h"

lmLedMatrix *matrix;
screen_t current_screen;
void *current_user_data;

pthread_t screen_thread;
pthread_mutex_t screen_mutex;
pthread_cond_t screen_cond;

static int running = 1;

typedef struct screen {
    screen_t screen;
    char name[32];
    UT_hash_handle hh;
} screen_st;

static screen_st *screens = NULL;


static void *start(void *ptr) {
    struct timespec current;
    struct timespec last_time;

    clock_gettime(CLOCK_MONOTONIC_RAW, &last_time);

    while (running) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &current);
        last_time = current;

        pthread_mutex_lock(&screen_mutex);
        while (current_screen == NULL) {
            pthread_cond_wait(&screen_cond, &screen_mutex);
        }
        pthread_mutex_unlock(&screen_mutex);


        current_screen(matrix, 0.12, current_user_data);

        sleep_nanos(20833300);
    }

    return NULL;
}


void screens_start(lmLedMatrix *matrix_) {
    pthread_create(&screen_thread, NULL, start, NULL);
    pthread_cond_init(&screen_cond, NULL);
    pthread_mutex_init(&screen_mutex, NULL);
    matrix = matrix_;
}

void screens_stop() {
    running = 0;
}

inline screen_t get_current_screen() {
    return current_screen;
}

screen_t set_current_screen(start_screen screen, void *user_data) {
    screen_t previous = current_screen;

    pthread_mutex_lock(&screen_mutex);
    current_screen = screen;
    current_user_data = user_data;
    pthread_cond_signal(&screen_cond);
    pthread_mutex_unlock(&screen_mutex);

    return previous;
}

screen_t get_screen(const char *name) {
    screen_st *st;

    HASH_FIND_STR(screens, name, st);

    if (st == NULL) {
        return NULL;
    }

    return st->screen;
}

void register_screen(const char *name, screen_t screen) {
    char *key = malloc(sizeof(const char) * strlen(name) + 1);
    strcpy(key, name);

    screen_st *st = malloc(sizeof(screen_st));
    st->screen = screen;
    strcpy(st->name, name);

    HASH_ADD_STR(screens, name, st);
}

void close_screens() {
    running = 0;

    pthread_cond_destroy(&screen_cond);
    pthread_mutex_destroy(&screen_mutex);
    pthread_detach(screen_thread);
}
