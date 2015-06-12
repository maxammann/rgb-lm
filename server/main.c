#include <lm/lm.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "controller.h"
#include "server.h"

#include "discovery/discovery_server.h"

#include "screen/screen.h"
#include "screen/menu.h"
#include "screen/visualize.h"

#include "alarms.h"
#include "wakedog.h"
#include "audio/audio.h"

#include "lm/ppm.h"
#include "rotary_encoder.h"

static int main_fd;
static int running;

void *start_discovery(void *nil) {
    printf("Starting discovery server\n");
    start_discovery_server();
    return NULL;
}

void *start_main_server(void *nil) {
    printf("Starting server\n");
    bind_tcp_socket(6969, &main_fd);
    start_server(main_fd, process_buffer);
    return NULL;
}

void shutdown(int sig) {
    printf("Stopping!\n");

    stop_discovery_server();
    stop_server(main_fd);

    screens_stop();
    lm_thread_stop(get_thread());

    running = 0;

    printf("Stopped!\n");
}

int main(int argc, char *argv[]) {
    printf("Initialising controller\n");
    register_menu_screens();
    register_visualize_screen();
    init_controller();


    image_t boot;
    ppm_load("alarm-clock/graphics/boot.ppm", &boot);
    ppm_render(get_matrix(), 0, 0, &boot);
    lm_matrix_swap_buffers(get_matrix());
    lm_thread_unpause(get_thread());

    // START INIT
    signal(SIGINT, shutdown);
    signal(SIGTERM, shutdown);

    printf("Initialising audio\n");
    audio_init();

    printf("Reading alams\n");
    read_alarms("test.alarms");

    setupencoder(9, 16, 15);

    pthread_t pthread;
    // Set minimum priority
    struct sched_param p;
    p.sched_priority = sched_get_priority_min(SCHED_FIFO);

    pthread_create(&pthread, NULL, start_discovery, NULL);
    pthread_setschedparam(pthread, SCHED_FIFO, &p);
    pthread_create(&pthread, NULL, start_main_server, NULL);
    pthread_setschedparam(pthread, SCHED_FIFO, &p);

    // END INIT
    set_current_screen(NULL, NULL);
    lm_thread_pause(get_thread());

    printf("Wakeup playlist: %s\n", getenv("WAKEUP_PLAYLIST"));
    printf("News playlist: %s\n", getenv("NEWS_PLAYLIST"));

    running = 1;

//    set_current_screen(get_screen("menu"), NULL);
//    lm_thread_unpause(get_thread());
//    menu_next();

    while (running) {
        watch();


        if (last_down_longer_than(1000)) {
            printf("Button long pressed\n");
            set_current_screen(NULL, NULL);
            lm_thread_pause(get_thread());

            reset_last_down();
        }

        if (last_up_longer_than(10000) && last_rotated_longer_than(10000)) {
            set_current_screen(NULL, NULL);
            lm_thread_pause(get_thread());

            reset_last_up();
        }

        sleep(1);
    }

    free_controller();
    close_screens();
    return 0;
}
