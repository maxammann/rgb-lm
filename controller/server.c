#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include "lm/lm.h"
#include "lm.pb-c.h"

#define TO_RGB(net_rgb) {(uint8_t) net_rgb->r, (uint8_t) net_rgb->g, (uint8_t) net_rgb->b};

lmLedMatrix *matrix;
lmThread *thread;
lmFontLibrary *library;


uint32_t size = 0;
u_int8_t *protoBuffer = NULL;

uint32_t buffer_start = 0;
uint32_t bytes_read = 0;

void fill(Lm__Fill *fill) {
    rgb rgb = TO_RGB(fill->rgb);
    lm_matrix_fill(matrix, &rgb);
    lm_matrix_swap_buffers(matrix);
}

void set_pixel(Lm__SetPixel *set_pixel) {
    rgb rgb = TO_RGB(set_pixel->rgb);
    Lm__Position *position = set_pixel->pos;
    lm_matrix_set_pixel(matrix, (uint16_t) position->x, (uint16_t) position->y, &rgb);
}


static void processBuffer() {
    Lm__Request *request = lm__request__unpack(NULL, size, (uint8_t const *) protoBuffer);


    switch (request->type) {
        case LM__REQUEST__TYPE__SETPIXEL:
            set_pixel(request->setpixel);
            break;
        case LM__REQUEST__TYPE__FILL:
            fill(request->fill);
            break;
        case LM__REQUEST__TYPE__CREATEFONT:
            break;
        case LM__REQUEST__TYPE__DESTROYFONT:
            break;
        case LM__REQUEST__TYPE__PRINTSTRING:
            break;
        case LM__REQUEST__TYPE__CREATESTRING:
            break;
        case LM__REQUEST__TYPE__DESTROYSTRING:
            break;
        case LM__REQUEST__TYPE__POPULATESTRING:
            break;
        case LM__REQUEST__TYPE__RENDERSTRING:
            break;
        case LM__REQUEST__TYPE__SWAPBUFFERS:
            lm_matrix_swap_buffers(matrix);
            break;
    }

    printf("Type: %d\n", request->type);
}

static void reset() {
    buffer_start = 0;
    bytes_read = 0;
    size = 0;

    free(protoBuffer);
}

static int bind_tcp_socket(uint16_t port) {
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

static int bind_unix_domain_socket(const char *socket_path) {
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


static void start_server(int fd) {
    u_int8_t buf[5]; // sizeof(buf) > sizeof(uint32_t)
    int cl;
    ssize_t rc = 0;

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

                protoBuffer = malloc(size);

                buffer_start = sizeof(uint32_t);
            }

            bytes_read += rc - buffer_start;

            memcpy(protoBuffer + already_written, buf + buffer_start, bytes_read);

            if (bytes_read == size) {
                processBuffer();
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

int main(int argc, char *argv[]) {
    lm_gpio_init();
    lm_gpio_init_output(lm_io_bits_new());

    matrix = lm_matrix_new(32, 32, 11);
    thread = lm_thread_new(matrix, DEFAULT_BASE_TIME_NANOS);
    library = lm_fonts_init();

//    lm_thread_pause(thread);
    lm_thread_start(thread);


//    int fd = bind_unix_domain_socket("./socket");
    int fd = bind_tcp_socket(6969);
    start_server(fd);


    lm_matrix_free(matrix);
    lm_thread_free(thread);
    lm_fonts_free(library);
    return 0;
}
