#include <lm/lm.h>

#include "controller.h"
#include "server.h"

#include "discovery/discovery_server.h"

#include "screen/screen.h"
#include "screen/example.h"
#include "screen/menu.h"
#include "screen/alarms.h"
#include "rotary_encoder.h"
#include "audio.h"

void *discovery(void *nil) {
    start_discovery_server();
    return NULL;
}

int main(int argc, char *argv[]) {

    init_controller();


    setupencoder(15, 16);

    register_example_screens();
    register_menu_screens();
    register_alarms_screens();


//    lm_thread_unpause(get_thread());
    set_current_screen(get_screen("menu"), NULL);

    pthread_t pthread;
    pthread_create(&pthread, NULL, discovery, NULL);

    pthread_t audio;
    pthread_create(&audio, NULL, (void *(*)(void *)) play, "test.ogg");


//    int fd = bind_unix_domain_socket("./socket");
    int fd = bind_tcp_socket(6969);
    start_server(fd, process_buffer);

    free_controller();
    close_screens();
    return 0;
}
