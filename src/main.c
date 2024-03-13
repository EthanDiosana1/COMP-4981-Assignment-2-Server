#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT_BASE 10
#define NUM_ARGS 2
// #define DEFAULT_BACKLOG 5
#define DEFAULT_BUFFER_SIZE 1024

/** @brief Displays how to use the program */
void display_usage(void);

uint16_t convert_port(const char *port_str);

int start_server(uint16_t port);

int readFrom(int sockfd);

int main(int argc, const char *argv[])
{
    uint16_t port;    // The port to run on

    if(argc != NUM_ARGS)
    {
        display_usage();
        return EXIT_FAILURE;
    }

    port = convert_port(argv[1]);
    printf("port:%hu\n", port);

    return EXIT_SUCCESS;
}

void display_usage(void)
{
    printf("---SSH CLIENT: SERVER SIDE---\n");
    printf("./main <port>\n");
}

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
    //
    // TODO: error checking
    // see client version

    return (uint16_t)port_ulong;
}

int start_server(uint16_t port)
{
    int                server_fd;
    int                client_fd;
    int                bind_result;    // The result of binding the socket
    int                listen_result;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t          client_addr_len;
    char               buffer[DEFAULT_BUFFER_SIZE];
    ssize_t            bytes_received;    // The bytes received from the connection

    client_addr_len = sizeof(client_addr);

    // Create the socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd == -1)
    {
        fprintf(stderr, "socket creation failed.\n");
        return EXIT_FAILURE;
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(port);

    // Set up the client address
    memset(&client_addr, 0, sizeof(client_addr));

    // Bind the socket
    bind_result = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if(bind_result == -1)
    {
        fprintf(stderr, "socket bind failed.\n");
        close(server_fd);
        return EXIT_FAILURE;
    }

    // Listen for connections
    listen_result = listen(server_fd, 1);

    if(listen_result == -1)
    {
        fprintf(stderr, "socket listen failed.\n");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Server listening to port %hu...\n", port);

    // Accept incoming connection
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

    if(client_fd == -1)
    {
        fprintf(stderr, "accept client failed\n");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Receive data from the client:
    while((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Received: %s", buffer);
    }

    if(bytes_received == -1)
    {
        fprintf(stderr, "recv failed\n");
        close(server_fd);
        return EXIT_FAILURE;
    }

    // Close the client socket.
    close(client_fd);

    // Close the server socket.
    close(server_fd);

    return EXIT_SUCCESS;
}
