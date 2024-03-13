#include "socket_server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_BACKLOG 5

// #define DEFAULT_BUFFER_SIZE 1024

/**
 * @brief Socket server client struct.
 */
struct client
{
    struct sockaddr_in client_address;
    socklen_t          client_length;
    int                client_fd;
};

struct socket_server create_socket_server_struct(const char *ip, const uint16_t port)
{
    struct socket_server server;
    unsigned long        ipLength;
    char                *ipCopy;

    // Malloc a copy of the ip.
    ipLength         = strlen(ip);
    ipCopy           = (char *)malloc(sizeof(char) * (ipLength + 1));
    ipCopy[ipLength] = '\n';

    // Make a copy of the ip.
    strncpy(ipCopy, ip, ipLength);

    server.ip   = ipCopy;
    server.port = port;

    // Free the malloced string.
    free(ipCopy);

    return server;
}

int start_socket_server(struct socket_server server)
{
    int                sockfd;
    struct sockaddr_in server_address;
    struct client      client;

    // Create the socket.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("Error opening socket.\n");
        return EXIT_FAILURE;
    }

    // Initialize server address.
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family      = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server.ip);
    server_address.sin_port        = htons(server.port);

    // Bind the socket to the server address.
    if(bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("bind() failed.\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("Server started! Listening...\n");

    // Listen for incoming connections.
    listen(sockfd, DEFAULT_BACKLOG);

    // Accept a connection.
    client.client_length = sizeof(client.client_address);

    client.client_fd = accept(sockfd, (struct sockaddr *)&client.client_address, &client.client_length);

    if(client.client_fd < 0)
    {
        perror("accept() failed.");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("Client accepted!\n");

    // Print out whatever the client sent.

    // Close the sockets.
    close(client.client_fd);
    close(sockfd);

    // Clear the server.

    return EXIT_SUCCESS;
}

void print_socket_server(struct socket_server server)
{
    printf("struct socket_server"
           "\n{"
           "\n\t ip = %s"
           "\n\t port = %d"
           "\n}",
           server.ip,
           server.port);
}
