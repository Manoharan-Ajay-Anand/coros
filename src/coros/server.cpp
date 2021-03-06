#include "server.h"
#include "socket.h"
#include "app.h"
#include "async/thread_pool.h"
#include "event/event.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <mutex>

coros::Server::Server(short port, ServerApplication& server_app,
                      event::SocketEventMonitor& event_monitor, async::ThreadPool& thread_pool) 
        : service(std::to_string(port)), server_app(server_app),
          event_monitor(event_monitor), thread_pool(thread_pool) {
}

addrinfo* coros::Server::get_local_addr_info() {
    int status;
    addrinfo* res;
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    status = getaddrinfo(NULL, service.c_str(), &hints, &res);
    if (status != 0) {
        throw std::runtime_error(std::string("Get Addr info: ").append(gai_strerror(status)));
    }
    return res;
}

void coros::Server::set_non_blocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL);
    int status = fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    if (status == -1) {
        throw std::runtime_error("Failed to set non-blocking");
    }
}

void coros::Server::setup() {
    addrinfo* info = get_local_addr_info(); 
    server_socketfd = socket(
        info->ai_family, info->ai_socktype, info->ai_protocol
    );
    if (server_socketfd == -1) {
        throw std::runtime_error(std::string("Server socket(): ").append(strerror(errno)));
    }
    int status = bind(server_socketfd, info->ai_addr, info->ai_addrlen);
    freeaddrinfo(info);
    if (status == -1) {
        throw std::runtime_error(std::string("Server bind(): ").append(strerror(errno)));
    }
    int yes = 1;
    if (setsockopt(server_socketfd , SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
        throw std::runtime_error(std::string("Server setsockopt(): ").append(strerror(errno)));
    } 
    set_non_blocking(server_socketfd);
    status = listen(server_socketfd, 10);
    if (status == -1) {
        throw std::runtime_error(std::string("Server listen(): ").append(strerror(errno)));
    }
    event_monitor.register_socket(server_socketfd, *this);
    event_monitor.listen_for_io(server_socketfd);
}

void coros::Server::on_socket_event(bool can_read, bool can_write) {
    if (can_read) {
        thread_pool.run([&] {
            sockaddr_storage client_addr;
            socklen_t addr_size = sizeof client_addr;
            int socket_fd = accept(
                server_socketfd, (sockaddr *) &client_addr, &addr_size
            );
            if (socket_fd == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    return;
                }
                throw std::runtime_error(std::string("Server accept(): ").append(strerror(errno)));
            }
            set_non_blocking(socket_fd);
            SocketDetails details { socket_fd, client_addr, addr_size };
            server_app.handle_socket(
                *this,
                std::make_shared<Socket>(details, event_monitor, thread_pool)
            );
        });
    }
    event_monitor.listen_for_io(server_socketfd);
}

void coros::Server::start(bool start_async) {
    if (start_async) {
        return thread_pool.run([&] { event_monitor.start(); });
    }
    event_monitor.start();
}

void coros::Server::shutdown() {
    event_monitor.shutdown();
    server_app.shutdown();
    thread_pool.shutdown();
    close(server_socketfd);
}
