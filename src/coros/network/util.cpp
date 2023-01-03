#include "util.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

void coros::base::throw_socket_error(int status, std::string prefix) {
    if (status == -1) {
        throw std::runtime_error(prefix.append(strerror(errno)));
    }
}

void coros::base::set_non_blocking_socket(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL);
    int status = fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    throw_socket_error(status, "set_non_blocking_socket error: ");
}
