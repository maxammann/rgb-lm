#include "server.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef uint32_t request_size_t;

#define SIZE sizeof(request_size_t)

static int running;
pthread_mutex_t running_mutex;

int bind_tcp_socket(uint16_t port, int *out_fd) {
    struct sockaddr_in addr;
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("socket error");
        return -1;
    }

    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    int error = bind(fd, (struct sockaddr *) &addr, sizeof(addr));

    if (error == -1) {
        perror("bind error");
        return -1;
    }

    *out_fd = fd;

    return 0;
}

int bind_unix_domain_socket(const char *socket_path, int *out_fd) {
    struct sockaddr_un addr;
    int fd;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("socket error");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    unlink(socket_path);

    int error = bind(fd, (struct sockaddr *) &addr, sizeof(addr));

    if (error == -1) {
        return -1;
    }

    *out_fd = fd;

    return 0;
}

void stop_server(int fd) {
    running = 0;
    close(fd);
}

void start_server(int fd, void (*process)(uint8_t *, size_t)) {
    running = 1;
    pthread_mutex_init(&running_mutex, NULL);

    uint8_t buf[256]; // sizeof(buf) > sizeof(uint32_t)
    int cl;
    ssize_t rc = 0;


    uint8_t msg_buff[256];
    request_size_t msg_buff_size = 0;

    if (listen(fd, 5) == -1) {
        perror("listen error");
        close(fd);
        return;
    }

    while (running) {

        printf("Waiting for new connection...\n");

        if ((cl = accept(fd, NULL, NULL)) == -1) {
            printf("Socket closed!\n");
            break;
        }

        while ((rc = read(cl, buf, sizeof(buf))) > 0) {
            memcpy(msg_buff + msg_buff_size, buf, (size_t) rc);
            msg_buff_size += rc;


            while (1) {
                if (msg_buff_size < SIZE) {
                    // basically no data
                    break;
                }

                request_size_t size = ntohl(
                        msg_buff[0] + (msg_buff[1] << 8) + (msg_buff[2] << 16) + (msg_buff[3] << 24));

                if (size > 256 || size < 1) {
                    close(cl);
                    break;
                }

                if (msg_buff_size - SIZE < size) {
                    // not enough data
                    break;
                }

                process(msg_buff + SIZE, size);


                msg_buff_size -= SIZE + size;

                if (msg_buff_size > 0) {
                    memcpy(msg_buff, msg_buff + SIZE + size, msg_buff_size);
                } else {
                    break;
                }
            }
        }

        if (msg_buff_size > 0) {
            printf("data still in buffer! clearing\n");
        }

        if (rc == -1) {
            perror("Read Error!\n");
        } else if (rc == 0) {
#ifdef DEBUG
            printf("Connection to client closed!\n");
#endif
            close(cl);
        }

        msg_buff_size = 0;
    }

}
