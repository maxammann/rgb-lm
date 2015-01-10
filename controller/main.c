#include <lm/lm.h>
#include "controller.h"
#include "server.h"
#include "screen/screens.h"

int main(int argc, char *argv[]) {
    init_controller();
    register_screens();

    set_current_screen(get_screen("digital_clock"));

//    int fd = bind_unix_domain_socket("./socket");
    int fd = bind_tcp_socket(6969);
    start_server(fd, process_buffer);

    free_controller();
    return 0;
}
