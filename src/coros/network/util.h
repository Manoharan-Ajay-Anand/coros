#ifndef COROS_NETWORK_UTIL_H
#define COROS_NETWORK_UTIL_H

#include <string>

struct addrinfo;

namespace coros::base {
    void set_non_blocking_socket(int socket_fd);
}

#endif
