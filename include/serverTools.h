#ifndef SERVER_TOOLS_C
#define SERVER_TOOLS_C

#include <stdint.h>

/**
 * @brief Converts a char port to a uint16_t port.
 * @param port_str the port string
 */
uint16_t convert_port(const char *port_str);

#endif
