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
 */
int handle_connection(int client_fd);

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

/**
 * @brief Receives a message from the client.
 * @param client_fd the file descriptor of the client socket
 * @param message the message buffer
 * @param message_size the expected size of the message
 */
int receive_message(int client_fd, char **message, ssize_t message_size);

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
        free(*message);
        return EXIT_FAILURE;
    }

    // Add null terminator to message
    (*message)[bytes_received] = '\0';

    return EXIT_SUCCESS;
}

int handle_connection(int client_fd)
{
    char   *message = NULL;
    char    output[MAX_COMMAND_SIZE];
    ssize_t message_size = 0;

    if(receive_message(client_fd, &message, message_size) == EXIT_FAILURE)
    {
        fprintf(stderr, "receive_message() failed\n");
        return EXIT_FAILURE;
    }

    // Print the received message
    printf("Received: %s\n", message);

    // Execute command locally
    if(execute_command(message, output, sizeof(output)) == -1)
    {
        fprintf(stderr, "Error executing command: %s\n", message);
    }

    // Send output back to client
    if(send(client_fd, output, strlen(output), 0) == -1)
    {
        fprintf(stderr, "send() failed\n");
        free(message);
        return EXIT_FAILURE;
    }

    free(message);

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

        // Skip if connection failed
        if(client_fd == EXIT_FAILURE)
        {
            fprintf(stderr, "accept_connection() failed\n");
            continue;
        }

        // Create a child process to handle
        // the connection
        pid = fork();

        // Skip if child fork failed
        if(pid == -1)
        {
            fprintf(stderr, "fork() failed\n");
            close(client_fd);
            continue;
        }

        // Handle connection in child process
        if(pid == 0)
        {
            // Child process
            close(server_fd);
            if(handle_connection(client_fd) == EXIT_FAILURE)
            {
                fprintf(stderr, "handle_connection() failed\n");
            }
            close(client_fd);
            exit(EXIT_SUCCESS);
        }

        // Close connection in parent process
        else
        {
            // Parent process
            close(client_fd);
        }

        return EXIT_SUCCESS;
    }
}
