#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// My libraries
#include "serverTools.h"
#include "socketSSHServer.h"

#define NUM_ARGS 3
// #define DEFAULT_BACKLOG 5

#define MIN_PORT 3000
#define MAX_PORT 10000

/** @brief Displays how to use the program */
void display_usage(void);

int main(int argc, const char *argv[])
{
    const char *ip;      // The ip to run the server on
    uint16_t    port;    // The port to run the server on

    // If invalid number of args
    if(argc != NUM_ARGS)
    {
        display_usage();
        return EXIT_FAILURE;
    }

    // If IP not localhost
    // or valid IP format
    // return failure
    ip = argv[1];

    printf("Inputted IP: %s\n", ip);

    if(!is_valid_ip(ip))
    {
        fprintf(stderr, "invalid IP address\n");
        display_usage();
        return EXIT_FAILURE;
    }

    // Convert  port from string to number
    port = convert_port(argv[2]);

    // If the port is less than the min...
    if(port < MIN_PORT || port > MAX_PORT)
    {
        fprintf(stderr, "port must be between 3000 and 10000 inclusive.\n");
        return EXIT_FAILURE;
    }

    // Start the server on the given ip and port.
    start_server(ip, port);

    return EXIT_SUCCESS;
}

void display_usage(void)
{
    printf("---SSH CLIENT: SERVER SIDE---\n");
    printf("./main <ip> <port>\n");
}
