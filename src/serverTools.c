#include "serverTools.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT_BASE 10

uint16_t convert_port(const char *port_str)
{
    char         *endptr;
    unsigned long port_ulong;    // The port as an unsigned long

    if(port_str == NULL)
    {
        fprintf(stderr, "port_str is null\n");
        exit(EXIT_FAILURE);
    }

    port_ulong = strtoul(port_str, &endptr, PORT_BASE);

    // TODO: error checking
    // see client version

    if(*endptr != '\0')
    {
        fprintf(stderr, "Error: Invalid port number format.\n");
        return EXIT_FAILURE;
    }

    if(port_ulong == UINT16_MAX)
    {
        fprintf(stderr, "Error: Port number out of range.\n");
        return EXIT_FAILURE;
    }

    return (uint16_t)port_ulong;
}

int send_message_size(int client_fd, size_t message_size)
{
    if(send(client_fd, &message_size, sizeof(size_t), 0) == -1)
    {
        fprintf(stderr, "send() failed\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int send_message(int client_fd, const char *buffer, size_t message_size)
{
    if(buffer == NULL)
    {
        fprintf(stderr, "buffer is null\n");
        return EXIT_FAILURE;
    }

    if(send(client_fd, buffer, message_size, 0) == -1)
    {
        fprintf(stderr, "send() failed\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
