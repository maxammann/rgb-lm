#include <lm/lm.h>
#include <stdio.h>
#include <m3u.h>

#include "controller.h"
#include "server.h"

#include "discovery/discovery_server.h"

#include "screen/screen.h"
#include "screen/example.h"
#include "screen/menu.h"
#include "screen/alarms.h"
#include "rotary_encoder.h"
#include "alarms.h"
#include "wakedog.h"
#include "audio.h"

void *discovery(void *nil) {
    start_discovery_server();
    return NULL;
}


int main(int argc, char *argv[]) {

//    size_t amount;
//    Title *titles = m3u_read("test1.m3u", &amount);
//
//    int i;
//    for (i = 0; i < amount; ++i) {
//        printf("%s", titles[i].title_dest);
//    }
//
//    m3u_free(titles, amount);
//    return 0;
//    time_t t = time(NULL);
//    struct tm *gtm = localtime(&t);
//    time_t gt = mktime(gtm);
//
//    gtm->tm_hour;
//    printf("time: %s\n", gtm->tm_zone);
//    return 0;



    read_alarms("test.alarms");

    setupencoder(15, 16, 14, skip_current_playback);

    init_controller();

    register_example_screens();
    register_menu_screens();
    register_alarms_screens();


    start_dog();


//    lm_thread_unpause(get_thread());
//    set_current_screen(get_screen("menu"), NULL);

    pthread_t pthread;
    pthread_create(&pthread, NULL, discovery, NULL);

//    int fd = bind_unix_domain_socket("./socket");
    int fd = bind_tcp_socket(6969);
    start_server(fd, process_buffer);

    free_controller();
    close_screens();
    return 0;
}
