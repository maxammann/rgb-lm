#include <lm/lm.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include "controller.h"
#include "server.h"

#include "discovery/discovery_server.h"

#include "screen/screen.h"
#include "screen/example.h"
#include "screen/menu.h"
#include "screen/alarms.h"
#include "screen/visualize.h"
#include "rotary_encoder.h"
#include "alarms.h"
#include "wakedog.h"
#include "audio/audio.h"

static int fd;

void *discovery(void *nil) {
    start_discovery_server();
    return NULL;
}

void shutdown(int sig) {
    //probably won't help much, todo more cleanups
    printf("Stopping!\n");

    stop_discovery_server();
    stop_server(fd);
    screens_stop();

    lm_thread_stop(get_thread());
    printf("Stopped!\n");
    exit(0);
}

int main(int argc, char *argv[]) {

    signal(SIGINT, shutdown);

    audio_init();

    read_alarms("test.alarms");

    setupencoder(9, 16, 15, skip_current_playback);


    init_controller();

    register_example_screens();
    register_menu_screens();
    register_alarms_screens();
    register_visualize_screen();

    start_dog();

//    lm_thread_unpause(get_thread());
//    set_current_screen(get_screen("visualize"), NULL);
//    audio_play_default("../sin2.wav", 0, NULL);


    lm_thread_unpause(get_thread());
    set_current_screen(get_screen("menu"), NULL);

    pthread_t pthread;
    pthread_create(&pthread, NULL, discovery, NULL);

    bind_tcp_socket(6969, &fd);
    start_server(fd, process_buffer);

    free_controller();
    close_screens();
    return 0;
}
