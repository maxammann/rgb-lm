#include "discovery_server.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int running;

void stop_discovery_server() {
    running = 0;
}

void start_discovery_server() {
    running = 1;

    int server_s;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    struct in_addr client_ip_addr;
    socklen_t addr_len;
    char out_buf[4096];
    char in_buf[4096];
    ssize_t ret;


    server_s = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_s < 0) {
        printf("*** ERROR - socket() failed \n");
        exit(-1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(server_s, (struct sockaddr *) &server_addr,
               sizeof(server_addr));
    if (ret < 0) {
        printf("*** ERROR - bind() failed \n");
        exit(-1);
    }


    printf("Discovery alarm-clock started\n");
    while (running) {
        addr_len = sizeof(client_addr);
        ret = recvfrom(server_s, in_buf, sizeof(in_buf), 0,
                       (struct sockaddr *) &client_addr, &addr_len);
        if (ret < 0) {
            printf("*** ERROR - recvfrom() failed \n");
            exit(-1);
        }

        memcpy(&client_ip_addr, &client_addr.sin_addr.s_addr, 4);

        printf("IP address of client = %s  port = %d) \n", inet_ntoa(client_ip_addr),
               ntohs(client_addr.sin_port));


        strcpy(out_buf, "DISCOVERY_RESPONSE");
        ret = sendto(server_s, out_buf, (strlen(out_buf) + 1), 0,
                     (struct sockaddr *) &client_addr, sizeof(client_addr));
        if (ret < 0) {
            printf("*** ERROR - sendto() failed \n");
            exit(-1);
        }
    }


    ret = close(server_s);
    if (ret < 0) {
        printf("*** ERROR - close() failed \n");
        exit(-1);
    }
}
