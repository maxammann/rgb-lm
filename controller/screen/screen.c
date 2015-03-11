#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include "screen.h"
#include "math.h"

lmLedMatrix *matrix;
GHashTable *screens;
screen_t current_screen;
void *current_user_data;

pthread_t screen_thread;
pthread_mutex_t screen_mutex;
pthread_cond_t screen_cond;

int running = 1;


static void *start(void *ptr) {

    struct timespec last_time;
    clock_gettime(CLOCK_REALTIME, &last_time);

    while (running) {

        struct timespec current;
        clock_gettime(CLOCK_REALTIME, &current);

        double elapsed = fabs(last_time.tv_sec * 10E9 + last_time.tv_nsec - current.tv_sec * 10E9 + current.tv_nsec);

        last_time.tv_nsec = current.tv_nsec;
        last_time.tv_sec = current.tv_sec;

        pthread_mutex_lock(&screen_mutex);
        while (current_screen == NULL) {
            pthread_cond_wait(&screen_cond, &screen_mutex);
        }

        pthread_mutex_unlock(&screen_mutex);


        current_screen(matrix, elapsed / 10E9, current_user_data);

        struct timespec sleep, remaining;
        sleep.tv_sec = 0;
        sleep.tv_nsec = 33333333;

        nanosleep(&sleep, &remaining);
    }

    return NULL;
}


void init_screens(lmLedMatrix *matrix_) {
    screens = g_hash_table_new(g_str_hash, g_str_equal);
    pthread_create(&screen_thread, NULL, start, NULL);
    pthread_cond_init(&screen_cond, NULL);
    pthread_mutex_init(&screen_mutex, NULL);
    matrix = matrix_;
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
    return g_hash_table_lookup(screens, name);
}

int register_screen(const char *name, screen_t screen) {
    char *key = malloc(sizeof(const char) * strlen(name) + 1);
    strcpy(key, name);
    return g_hash_table_insert(screens, key, screen);
}

void close_screens() {
    running = 0;

    pthread_cond_destroy(&screen_cond);
    pthread_mutex_destroy(&screen_mutex);
    pthread_detach(screen_thread);
}
