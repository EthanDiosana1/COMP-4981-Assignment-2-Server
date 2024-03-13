#include "arguments.h"
#include "socket_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LOCALHOST "127.0.0.1"
#define DEFAULT_PORT 8000

struct arguments;
int                   display_usage(void);
struct socket_server *parse_arguments(int argc, char *argv[]);
int                   start_server(int argc, char *argv[]);

int main(const int argc, char *argv[])
{
    // Handle arguments.

    // Start the server.
    start_server(argc, argv);

    return EXIT_SUCCESS;
}

int start_server(int argc, char *argv[])
{
    struct socket_server *server;
    int                   server_start_status;

    server = parse_arguments(argc, argv);

    server_start_status = start_socket_server(*server);

    // Display error if server failed to start.
    if(server_start_status != EXIT_SUCCESS)
    {
        perror("Server failed to start.\n");
        return EXIT_FAILURE;
    }

    printf("Server started at %s:%d\n", LOCALHOST, DEFAULT_PORT);
    return EXIT_SUCCESS;
}

int display_usage(void)
{
    printf("arguments: -i <ip> -p <port>\n");
    return EXIT_SUCCESS;
}

struct socket_server *parse_arguments(int argc, char *argv[])
{
    int                   opt;       // the option char
    uint16_t              port;      // the port
    char                 *ip;        // the ip
    struct socket_server *server;    // the server

    // If argc < 2, display usage.
    if(argc < 2)
    {
        display_usage();
        return server;
    }

    // Parse the arguments out.
    while((opt = getopt(argc, argv, "p:i:")) != -1)
    {
        if(opt == 'p')
        {
            port = (uint16_t)atoi(optarg);
        }

        if(opt == 'i')
        {
            ip = optarg;
        }

        if(!opt)
        {
            break;
        }
    }

    server = (struct socket_server *)malloc(sizeof(struct socket_server));
    server = (struct socket_server *)create_socket_server_struct(ip, port);

    return server;
}
