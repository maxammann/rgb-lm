#pragma once

#include "stdint.h"
#include "stddef.h"

int bind_tcp_socket(uint16_t port);

int bind_unix_domain_socket(const char *socket_path);

void start_server(int fd, void (*process)(uint8_t *, size_t));
