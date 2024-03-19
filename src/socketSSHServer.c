#include "socketSSHServer.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX_COMMAND_SIZE 1024
#define COMMAND_FAILED_MESSAGE "command failed"

#include "commands.h"
#include "serverTools.h"

int start_server(const char *ip, uint16_t port)
{
    int                server_fd;
    int                bind_result;      // The result of binding the socket int listen_result;    // The result of listening for connections
    int                listen_result;    // The result of listening for a client connection
    int                enable;           // The enable for setsockopt.
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    // Create the socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd == -1)
    {
        fprintf(stderr, "socket creation failed.\n");
        return EXIT_FAILURE;
    }

    // Set the socket to be reusable
    enable = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
    {
        fprintf(stderr, "setsockopt() failed\n");
        close(server_fd);
        return EXIT_FAILURE;
    }
    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
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

        // Create a child process to handle                  // the connection
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
        {    // Parent process
            close(client_fd);
        }
    }
}

int handle_connection(int client_fd)
{
    while(1)
    {
        char   *message = NULL;
        char    output[MAX_COMMAND_SIZE];
        ssize_t message_size = 0;

        printf("\n----- LISTENING FOR CLIENT INPUT -----\n");

        if(receive_message(client_fd, &message, message_size) == EXIT_FAILURE)
        {
            fprintf(stderr, "receive_message() failed\n");
            return EXIT_FAILURE;
        }

        printf("\n----- CLIENT REQUEST START -----\n");

        // Print the received message
        printf("Received: %s\n", message);

        // Execute command locally
        if(execute_command(message, output, sizeof(output)) == EXIT_FAILURE)
        {
            fprintf(stderr, "Error executing command: %s\n", message);
            printf("Output: \"%s\"\n", output);
            strncpy(output, COMMAND_FAILED_MESSAGE, strlen(COMMAND_FAILED_MESSAGE));
        }

        // Send output back to client
        if(send_message(client_fd, output) == EXIT_FAILURE)
        {
            fprintf(stderr, "send() failed\n");
            free(message);
            return EXIT_FAILURE;
        }

        free(message);
        printf("----- CLIENT REQUEST END -----\n\n");
    }
}
