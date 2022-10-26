#include "monitor.h"
#include "handler.h"

#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <cerrno>
#include <poll.h>
#include <string>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <algorithm>

coros::event::SocketEventMonitor::SocketEventMonitor() {
    is_shutdown = false;
}

void coros::event::SocketEventMonitor::register_socket(int socket_fd, SocketEventHandler& handler) {
    std::lock_guard<std::mutex> handler_lock(handler_mutex);
    handler_map[socket_fd] = &handler;
}

void coros::event::SocketEventMonitor::deregister_socket(int socket_fd) {
    std::lock_guard<std::mutex> handler_lock(handler_mutex);
    handler_map.erase(socket_fd);
}

void coros::event::SocketEventMonitor::listen_for_io(int socket_fd) {
    {
        std::lock_guard<std::mutex> event_lock(event_mutex);
        event_set.insert(socket_fd);
    }
    event_condition.notify_one();
}

void coros::event::SocketEventMonitor::populate_events(std::vector<pollfd>& pollfds) {
    for (auto it = event_set.begin(); it != event_set.end(); ++it) {
        pollfd pfd { *it, POLLIN | POLLOUT, 0 };
        pollfds.push_back(pfd);
    }
    event_set.clear();
}

void coros::event::SocketEventMonitor::trigger_events(std::vector<pollfd>& pollfds) {
    std::lock_guard<std::mutex> handler_lock(handler_mutex);
    for (auto it = pollfds.begin(); it != pollfds.end(); ++it) {
        pollfd& pfd = *it;
        bool can_read = (pfd.revents & POLLIN) == POLLIN;
        bool can_write = (pfd.revents & POLLOUT) == POLLOUT;
        auto handler_it = handler_map.find(pfd.fd);
        if (handler_it != handler_map.end()) {
            handler_it->second->on_socket_event(can_read, can_write);
        }
    }
    pollfds.clear();
}

void coros::event::SocketEventMonitor::start() {
    std::vector<pollfd> pollfds;
    while (true) {
        {
            std::unique_lock<std::mutex> event_lock(event_mutex);
            event_condition.wait(event_lock, [&] { 
                return !event_set.empty() || is_shutdown; 
            });
            if (is_shutdown) {
                return;
            }
            populate_events(pollfds);
        }
        int fd_count = poll(pollfds.data(), pollfds.size(), POLL_TIMEOUT);
        if (fd_count == -1) {
            throw std::runtime_error(std::string("Poll Error: ").append(strerror(errno)));
        }
        trigger_events(pollfds);
    }
}

void coros::event::SocketEventMonitor::shutdown() {
    {
        std::lock_guard<std::mutex> event_lock(event_mutex);
        if (is_shutdown) {
            throw std::runtime_error("SocketEventMonitor shutdown() error: already shutdown");
        }
        is_shutdown = true;
    }
    event_condition.notify_one();
}
