#include "server.h"
#include "socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <stdexcept>
#include <iostream>
#include <memory>

server::Server::Server(short port) : service(std::to_string(port)) {
}

server::Server::~Server() {
    shutdown(server_socketfd, SHUT_RDWR);
}

addrinfo server::Server::get_local_addr_info() {
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
    return *res;
}

void server::Server::set_non_blocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL);
    int status = fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    if (status == -1) {
        throw std::runtime_error("Failed to set non-blocking");
    }
}

void server::Server::bootstrap() {
    addrinfo info = get_local_addr_info(); 
    server_socketfd = socket(
        info.ai_family, info.ai_socktype, info.ai_protocol
    );
    if (server_socketfd == -1) {
        throw std::runtime_error(std::string("Server socket(): ").append(strerror(errno)));
    }
    int status = bind(server_socketfd, info.ai_addr, info.ai_addrlen);
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
    event_monitor.register_handler(server_socketfd, this);
    event_monitor.listen_for_event(server_socketfd, true, false);
    event_monitor.start();
}

void server::Server::on_socket_event(bool can_read, bool can_write) {
    if (can_read) {
        sockaddr_storage client_addr;
        socklen_t addr_size = sizeof client_addr;
        int socket_fd = accept(
            server_socketfd, (sockaddr *) &client_addr, &addr_size
        );
        if (socket_fd == -1) {
            throw std::runtime_error(std::string("Server accept(): ").append(strerror(errno)));
        }
        std::cerr << "Socket connection detected: " << socket_fd << std::endl;
        set_non_blocking(socket_fd);
        socket_map[socket_fd] = std::make_unique<Socket>(socket_fd, client_addr, addr_size, *this, 
            event_monitor, thread_pool);
        thread_pool.execute([&, socket_fd] { socket_map.at(socket_fd)->handle_request(); });
    }
    event_monitor.listen_for_event(server_socketfd, true, false);
}