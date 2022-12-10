#include "monitor.h"
#include "handler.h"

#include <vector>
#include <unordered_map>
#include <mutex>
#include <cerrno>
#include <sys/epoll.h>
#include <string>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

coros::base::SocketEventMonitor::SocketEventMonitor() {
    is_shutdown = false;
    epoll_fd = epoll_create(100);
    if (epoll_fd < 0) {
        throw std::runtime_error(std::string("SocketEventMonitor: ").append(strerror(errno))); 
    }
}

void coros::base::SocketEventMonitor::register_socket(int socket_fd, SocketEventHandler& handler) {
    std::lock_guard<std::mutex> handler_lock(handler_mutex);
    handler_map[socket_fd] = &handler;
    epoll_event e_event;
    e_event.data.fd = socket_fd;
    int status = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &e_event);
    if (status < 0) {
        throw std::runtime_error(std::string("SocketEventMonitor register: ").append(strerror(errno))); 
    } 
}

void coros::base::SocketEventMonitor::deregister_socket(int socket_fd) {
    std::lock_guard<std::mutex> handler_lock(handler_mutex);
    handler_map.erase(socket_fd);
    epoll_event e_event;
    e_event.data.fd = socket_fd;
    int status = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, &e_event);
    if (status < 0) {
        throw std::runtime_error(std::string("SocketEventMonitor deregister: ").append(strerror(errno))); 
    }
}

void coros::base::SocketEventMonitor::listen_for_io(int socket_fd) {
    epoll_event e_event;
    e_event.data.fd = socket_fd;
    e_event.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
    int status = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd, &e_event);
    if (status < 0) {
        throw std::runtime_error(std::string("SocketEventMonitor listen: ").append(strerror(errno))); 
    }
}

void coros::base::SocketEventMonitor::trigger_events(std::vector<epoll_event>& events, int count) {
    std::lock_guard<std::mutex> handler_lock(handler_mutex);
    for (int i = 0; i < count; i++) {
        epoll_event& event = events[i];
        bool can_read = (event.events & EPOLLIN) == EPOLLIN;
        bool can_write = (event.events & EPOLLOUT) == EPOLLOUT;
        auto handler_it = handler_map.find(event.data.fd);
        if (handler_it != handler_map.end()) {
            handler_it->second->on_socket_event(can_read, can_write);
        }
    }
}

void coros::base::SocketEventMonitor::start() {
    std::vector<epoll_event> events(POLL_MAX_EVENTS);
    while (!is_shutdown) {
        int count = epoll_wait(epoll_fd, events.data(), POLL_MAX_EVENTS, POLL_TIMEOUT);
        if (count < 0) {
            throw std::runtime_error(std::string("SocketEventMonitor start: ").append(strerror(errno)));
        }
        trigger_events(events, count);
    }
}

void coros::base::SocketEventMonitor::shutdown() {
    if (is_shutdown.exchange(true)) {
        throw std::runtime_error("SocketEventMonitor shutdown() error: already shutdown");
    }
    close(epoll_fd);
}
