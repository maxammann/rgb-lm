#include "server.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

int bind_tcp_socket(uint16_t port) {
    struct sockaddr_in addr;
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("socket error");
        exit(-1);
    }

    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    int error = bind(fd, (struct sockaddr *) &addr, sizeof(addr));

    if (error == -1) {
        perror("bind error");
        exit(-1);
    }

    return fd;
}

int bind_unix_domain_socket(const char *socket_path) {
    struct sockaddr_un addr;
    int fd;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("socket error");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    unlink(socket_path);

    int error = bind(fd, (struct sockaddr *) &addr, sizeof(addr));

    if (error == -1) {
        perror("bind error");
        exit(-1);
    }

    return fd;
}

void start_server(int fd, void (process)(u_int8_t *, size_t)) {
    u_int8_t buf[5]; // sizeof(buf) > sizeof(uint32_t)
    int cl;
    ssize_t rc = 0;


    uint32_t size = 0;
    u_int8_t *target_buffer = NULL;
    uint32_t buffer_start = 0;
    uint32_t bytes_read = 0;

    void reset() {
        buffer_start = 0;
        bytes_read = 0;
        size = 0;

        free(target_buffer);
    }

    if (listen(fd, 5) == -1) {
        perror("listen error");
        exit(-1);
    }

    while (1) {
        if ((cl = accept(fd, NULL, NULL)) == -1) {
            perror("accept error");
            continue;
        }

        while ((rc = read(cl, buf, sizeof(buf))) > 0) {

            int already_written = bytes_read;

            if (bytes_read == 0) {
                size = ntohl(buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24));

                target_buffer = malloc(size);

                buffer_start = sizeof(uint32_t);
            }

            bytes_read += rc - buffer_start;

            memcpy(target_buffer + already_written, buf + buffer_start, bytes_read);

            if (bytes_read == size) {
                process(target_buffer, size);
                reset();
            }

            buffer_start = 0;
        }

        if (rc == -1) {
            perror("read");
            exit(-1);
        } else if (rc == 0) {
            printf("Closed connection");
            close(cl);
        }
    }

}
