#ifndef COROS_NETWORK_UTIL_H
#define COROS_NETWORK_UTIL_H

#include <string>

struct addrinfo;

namespace coros::base {
    void throw_socket_error(int status, std::string prefix);

    void set_non_blocking_socket(int socket_fd);
}

#endif
