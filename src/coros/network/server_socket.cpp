#include "server_socket.h"
#include "util.h"

#include "coros/awaiter/accept_awaiter.h"
#include "coros/event/manager.h"

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

addrinfo* get_local_addr_info(short port) {
    std::string service = std::to_string(port);
    int status;
    addrinfo* res;
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    status = getaddrinfo(NULL, service.c_str(), &hints, &res);
    coros::base::throw_socket_error(status, "get_addr_info: ");
    return res;
}

coros::base::ServerSocket::ServerSocket(short port, ThreadPool& thread_pool, 
                                        SocketEventMonitor& event_monitor)
        : event_manager(event_monitor, thread_pool), thread_pool(thread_pool), 
          event_monitor(event_monitor) {
    addrinfo* info = get_local_addr_info(port); 
    socket_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    throw_socket_error(socket_fd, "ServerSocket socket(): ");
    int status = bind(socket_fd, info->ai_addr, info->ai_addrlen);
    freeaddrinfo(info);
    throw_socket_error(status, "ServerSocket bind(): ");
    int yes = 1;
    status = setsockopt(socket_fd , SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    throw_socket_error(status, "ServerSocket setsockopt(): ");
    set_non_blocking_socket(socket_fd);
    status = listen(socket_fd, 10);
    throw_socket_error(status, "ServerSocket listen(): ");
    event_manager.register_socket_fd(socket_fd);
}

coros::base::SocketAcceptAwaiter coros::base::ServerSocket::accept() {
    return { socket_fd, thread_pool, event_manager, event_monitor };
}

void coros::base::ServerSocket::close_socket() {
    event_manager.close();
    close(socket_fd);
}
