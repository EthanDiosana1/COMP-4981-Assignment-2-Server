#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <stdint.h>

struct socket_server
{
    const char *ip;
    uint16_t    port;
};

struct socket_server create_socket_server_struct(const char *ip, uint16_t port);

int start_socket_server(struct socket_server server);

void print_socket_server(struct socket_server server);

#endif
