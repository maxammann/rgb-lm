#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lm.pb-c.h"

char *socket_path = "./socket";
//char *socket_path = "\0hidden";

int main(int argc, char *argv[]) {
    struct sockaddr_un addr;
    int fd;

//    if (argc > 1) socket_path=argv[1];

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("connect error");
        exit(-1);
    }

    Lm__Request request = LM__REQUEST__INIT;
    Lm__Fill fill = LM__FILL__INIT;
    Lm__RGB rgb = LM__RGB__INIT;

    rgb.r = 255;
    rgb.g = 0;
    rgb.b = 0;

    fill.rgb = &rgb;

    request.fill = &fill;
    request.type = LM__REQUEST__TYPE__FILL;

    size_t size = lm__request__get_packed_size(&request);
    u_int8_t *out = malloc(size);
    lm__request__pack(&request, out);

    uint32_t out_size = (uint32_t) size;
    write(fd, &out_size, sizeof(uint32_t));

    ssize_t out_write = write(fd, out, size);

    printf("%d\n", out_write);

    return 0;
}
