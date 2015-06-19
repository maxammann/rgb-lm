#pragma once

#include "stdint.h"
#include "stddef.h"

int bind_tcp_socket(uint16_t port, int *out_fd);

int bind_unix_domain_socket(const char *socket_path, int *out_fd);

void start_server(int fd, void (*process)(uint8_t *, size_t));

void stop_server(int fd);
