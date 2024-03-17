#ifndef SOCKET_SSH_SERVER
#define SOCKET_SSH_SERVER

#include <stdint.h>
#include <unistd.h>

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
 * @brief Starts the socket server
 * @param ip The ip to run the server on
 * @param port The port to run the server on
 */
int start_server(const char *ip, uint16_t port);

/**
 * @brief Handles a client connection
 * @param client_fd The file descriptor of the client socket
 */
int handle_connection(int client_fd);

#endif
