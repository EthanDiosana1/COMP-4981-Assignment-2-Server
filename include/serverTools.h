#ifndef SERVER_TOOLS_C
#define SERVER_TOOLS_C

#include <stdint.h>
#include <sys/socket.h>

/**
 * @brief Converts a char port to a uint16_t port.
 * @param port_str the port string
 */
uint16_t convert_port(const char *port_str);

/**
 * @brief Sends the size of the message to the server.
 * @param message_size the size of the message in bytes
 *
 */
int send_message_size(int client_fd, size_t message_size);

/**
 * @brief Sends the message to the server.
 * @param client_fd the client file descriptor
 * @param buffer the buffer containing the message
 * @param message_size the size of the message
 */
int send_message(int client_fd, const char *buffer, size_t message_size);

#endif
