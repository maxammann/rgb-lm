#include <lm/lm.h>
#include "controller.h"
#include "server.h"
#include "screen/example_screens.h"
#include "screen/menu_screens.h"
#include "screen/screen.h"

int main(int argc, char *argv[]) {
    init_controller();

    register_example_screens();
    register_menu_screens();

    lm_thread_unpause(get_thread());
    set_current_screen(get_screen("digital_clock"), NULL);

//    int fd = bind_unix_domain_socket("./socket");
    int fd = bind_tcp_socket(6969);
    start_server(fd, process_buffer);

    free_controller();
    close_screens();
    return 0;
}
