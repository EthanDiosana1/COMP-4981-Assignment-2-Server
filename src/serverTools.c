#include "serverTools.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT_BASE 10
#define IP_BASE 10
#define IP_MAX 255
#define IP_MAX_CHARS 16

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

bool isValidIp(const char *ip)
{
    const char *token;
    char       *saveptr;
    char        ip_copy[IP_MAX_CHARS];
    int         count;

    // Spit the IP into four parts.
    strlcpy(ip_copy, ip, IP_MAX_CHARS);
    count = 0;
    token = strtok_r(ip_copy, ".", &saveptr);
    while(token != NULL)
    {
        char *endptr;
        long  num;
        count++;
        num = strtol(token, &endptr, IP_BASE);

        if(*endptr != '\0' || num < 0 || num > IP_MAX)
        {
            fprintf(stderr, "Invalid ip address\n");
            return false;
        }

        token = strtok_r(NULL, ".", &saveptr);
    }

    // Return if the address has exactly four parts
    if(count != 4)
    {
        return false;
    }

    return true;
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

int send_message_content(int client_fd, const char *buffer, size_t message_size)
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

int send_message(int client_fd, char *buffer)
{
    size_t buffer_size;

    // Remove the newline
    buffer_size = strlen(buffer);

    buffer[buffer_size] = '\0';

    // If the buffer size is 0, do not send.
    if(buffer_size == 0)
    {
        return EXIT_FAILURE;
    }

    // Send the message size to the server
    send_message_size(client_fd, buffer_size);

    // Send the message content to the server
    send_message_content(client_fd, buffer, buffer_size);

    return EXIT_SUCCESS;
}

int receive_message(int client_fd, char **message, ssize_t message_size)
{
    ssize_t bytes_received;

    // Receive the message size
    bytes_received = recv(client_fd, &message_size, sizeof(ssize_t), 0);
    if(bytes_received <= 0)
    {
        // Handle client disconnect
        if(bytes_received == 0)
        {
            printf("Client disconnected.\n");
        }
        else
        {
            fprintf(stderr, "recv() failed\n");
        }
    }

    // Allocate memory for message buffer
    *message = malloc((size_t)(message_size) + 1);

    if(*message == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for message.\n");
        return EXIT_FAILURE;
    }

    // Receive message from client.
    bytes_received = recv(client_fd, *message, (size_t)message_size, 0);
    if(bytes_received <= 0)
    {
        if(bytes_received == 0)
        {
            printf("Client disconnected.\n");
        }
        else
        {
            fprintf(stderr, "recv() failed\n");
        }
        free(*message);
        return EXIT_FAILURE;
    }

    // Add null terminator to message
    (*message)[bytes_received] = '\0';

    return EXIT_SUCCESS;
}
