#include "event.h"

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

coros::event::SocketEventMonitor::SocketEventMonitor() {
    is_shutdown = false;
}

void coros::event::SocketEventMonitor::register_socket(int socket_fd, SocketHandler* handler) {
    std::lock_guard<std::mutex> handler_lock(handler_mutex);
    handler_map[socket_fd] = handler;
}

void coros::event::SocketEventMonitor::deregister_socket(int socket_fd) {
    std::lock_guard<std::mutex> socket_lock(handler_mutex);
    handler_map.erase(socket_fd);
}

void coros::event::SocketEventMonitor::listen_for_read(int socket_fd) {
    {
        std::lock_guard<std::mutex> event_lock(event_mutex);
        event_queue.push({ socket_fd, true, false });
    }
    event_condition.notify_one();
}

void coros::event::SocketEventMonitor::listen_for_write(int socket_fd) {
    {
        std::lock_guard<std::mutex> event_lock(event_mutex);
        event_queue.push({ socket_fd, false, true });
    }
    event_condition.notify_one();
}

void set_event(coros::event::SocketEvent& socket_event, pollfd& pollfd) {
    if (socket_event.do_read) {
        pollfd.events = pollfd.events | POLLIN;
    }
    if (socket_event.do_write) {
        pollfd.events = pollfd.events | POLLOUT;
    }
}

void coros::event::SocketEventMonitor::populate_events(std::vector<pollfd>& pollfds) {
    std::unique_lock<std::mutex> event_lock(event_mutex);
    event_condition.wait(event_lock, [&] { return !event_queue.empty() || is_shutdown; });
    std::unordered_map<int , pollfd*> pollfd_map;
    while (!event_queue.empty() && !is_shutdown) {
        SocketEvent& event = event_queue.front();
        auto it = pollfd_map.find(event.socket_fd);
        if (it != pollfd_map.end()) {
            set_event(event, *(it->second));
        } else {
            pollfd pfd { event.socket_fd, 0, 0 } ;
            set_event(event, pfd);
            pollfds.push_back(pfd);
            pollfd_map[event.socket_fd] = &(pollfds.back());
        }
        event_queue.pop();
    }
}

void coros::event::SocketEventMonitor::trigger_events(std::vector<pollfd>& pollfds) {
    for (auto it = pollfds.begin(); it != pollfds.end() && !is_shutdown; it++) {
        pollfd& pfd = *it;
        bool can_read = (pfd.revents & POLLIN) == POLLIN;
        bool can_write = (pfd.revents & POLLOUT) == POLLOUT;
        std::lock_guard<std::mutex> handler_lock(handler_mutex);
        auto handler_it = handler_map.find(pfd.fd);
        if (handler_it != handler_map.end()) {
            handler_it->second->on_socket_event(can_read, can_write);
        }
    } 
}

void coros::event::SocketEventMonitor::start() {
    while (!is_shutdown) {
        std::vector<pollfd> pollfds;
        populate_events(pollfds);
        int fd_count = poll(pollfds.data(), pollfds.size(), 2000);
        if (fd_count == -1) {
            throw std::runtime_error(std::string("Poll Error: ").append(strerror(errno)));
        }
        trigger_events(pollfds);
    }
}

void coros::event::SocketEventMonitor::shutdown() {
    if (is_shutdown) {
        throw std::runtime_error("SocketEventMonitor shutdown() error: already shutdown");
    }
    is_shutdown = true;
    event_condition.notify_one();
}
