#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// My libraries
#include "commands.h"
#include "serverTools.h"

#define MAX_COMMAND_SIZE 1024

#define NUM_ARGS 2
// #define DEFAULT_BACKLOG 5

/** @brief Displays how to use the program */
void display_usage(void);

int start_server(uint16_t port);

/**
 * @brief Defines the behavior of a connection
 * @param client_fd The client socket file descriptor
 * @param server_fd The server socket file descriptor
 */
int handle_connection(int client_fd, int server_fd);

/**
 * @brief Accepts a single socket connection to the server.
 * @param server_fd The socket server file descriptor
 */
int accept_connection(int server_fd);

/**
 * @brief Accepts multiple connections using fork()
 * @param server_fd The socket server file descriptor
 */
int accept_connections(int server_fd);

int main(int argc, const char *argv[])
{
    uint16_t port;    // The port to run on

    if(argc != NUM_ARGS)
    {
        display_usage();
        return EXIT_FAILURE;
    }

    port = convert_port(argv[1]);
    printf("port: %hu\n", port);

    start_server(port);

    return EXIT_SUCCESS;
}

void display_usage(void)
{
    printf("---SSH CLIENT: SERVER SIDE---\n");
    printf("./main <port>\n");
}

int start_server(uint16_t port)
{
    int                server_fd;
    int                bind_result;      // The result of binding the socket
    int                listen_result;    // The result of listening for connections
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

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

    // Accept incoming connections
    accept_connections(server_fd);

    // Close the server socket.
    close(server_fd);

    return EXIT_SUCCESS;
}

int handle_connection(int client_fd, int server_fd)
{
    ssize_t bytes_received;    // The bytes received from the connection
    ssize_t message_size;      // The size of the received message.
    while(1)
    {
        size_t     received_size;
        char      *message;
        const char command[MAX_COMMAND_SIZE];
        char       output[MAX_COMMAND_SIZE];

        bytes_received = recv(client_fd, &message_size, sizeof(ssize_t), 0);
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
            break;
        }

        // Cast ssize_t to size_t
        received_size = (size_t)bytes_received;

        // Allocate memory for message buffer
        message = malloc(received_size + 1);

        if(message == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for message.\n");
            free(message);
            break;
        }

        // Receive message from client.
        bytes_received = recv(client_fd, message, (size_t)message_size, 0);
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
            free(message);
            break;
        }

        // Add null terminator to message
        message[bytes_received] = '\0';

        // Print the received message
        printf("Received: %s\n", message);

        // Execute command locally.
        if(execute_command(command, output, sizeof(message)) == -1)
        {
            fprintf(stderr, "Error executing command: %s\n", message);
        }

        // Send output back to client.
        if(send(client_fd, output, strlen(output), 0) == -1)
        {
            fprintf(stderr, "send() failed\n");
        }

        free(message);
    }

    if(bytes_received == -1)
    {
        fprintf(stderr, "recv failed\n");
        close(server_fd);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int accept_connection(int server_fd)
{
    int                client_fd;
    struct sockaddr_in client_addr;
    socklen_t          client_addr_len;

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr_len = sizeof(client_addr);

    // Accept an incoming connection
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if(client_fd == -1)
    {
        fprintf(stderr, "accept() failed\n");
        return EXIT_FAILURE;
    }

    printf("Connection accepted from: %s:%hu\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    return client_fd;
}

int accept_connections(int server_fd)
{
    while(1)
    {
        int client_fd;
        int pid;

        // Accept a single connection
        client_fd = accept_connection(server_fd);
        pid       = fork();
        if(pid == -1)
        {
            fprintf(stderr, "fork failed\n");
            continue;
        }

        if(pid == 0)
        {
            close(server_fd);

            handle_connection(client_fd, server_fd);

            close(client_fd);
        }

        else
        {
            // Close client socket in parent process.
            close(client_fd);
        }
    }
}
