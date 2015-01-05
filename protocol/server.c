#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include "lm/lm.h"
#include "lm.pb-c.h"

#define TO_RGB(net_rgb) {(uint8_t) net_rgb->r, (uint8_t) net_rgb->g, (uint8_t) net_rgb->b};

char *socket_path = "./socket";

lmLedMatrix *matrix;
lmThread *thread;
lmFontLibrary *library;


uint32_t size = 0;
u_int8_t *protoBuffer = NULL;

uint32_t buffer_start = 0;
uint32_t bytes_read = 0;

void fill(Lm__Fill *fill) {
    rgb rgb = TO_RGB(fill->rgb);
//    lm_matrix_fill(matrix, &rgb);
}

void set_pixel(Lm__SetPixel *set_pixel) {
    rgb rgb = TO_RGB(set_pixel->rgb);
    Lm__Position *position = set_pixel->pos;
//    lm_matrix_set_pixel(matrix, (uint16_t) position->x, (uint16_t) position->y, &rgb);
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
    if (protoBuffer != NULL) {
        free(protoBuffer);
    }
}

static void start_server() {
    struct sockaddr_un addr;
    u_int8_t buf[100];
    int fd, cl;
    ssize_t rc = 0;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    unlink(socket_path);

    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind error");
        exit(-1);
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
                size = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
                protoBuffer = malloc(size);

                buffer_start = sizeof(int32_t);
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
        }
        else if (rc == 0) {
            printf("EOF\n");
            close(cl);
        }
    }

}

int main(int argc, char *argv[]) {
//    lm_gpio_init();
//    lm_gpio_init_output(lm_io_bits_new());
//
//    lmLedMatrix *matrix = lm_matrix_new(32, 32, 11);
//    lmThread *thread = lm_thread_new(matrix, DEFAULT_BASE_TIME_NANOS);
//    lmFontLibrary *library = lm_fonts_init();
//
//    lm_thread_pause(thread);
//    lm_thread_start(thread);


    start_server();


//    lm_matrix_free(matrix);
//    lm_thread_free(thread);
//    lm_fonts_free(library);
    return 0;
}
