#include "event.h"
#include "server/server.h"

#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <cerrno>
#include <poll.h>
#include <string>
#include <cstring>
#include <iostream>

void event::SocketEventMonitor::register_handler(int socket_fd, SocketEventHandler* handler) {
    std::lock_guard<std::mutex> handler_lock(handler_mutex);
    handler_map[socket_fd] = handler;
    std::cerr << "Handler registered: " << socket_fd << std::endl;
}

void event::SocketEventMonitor::deregister_handler(int socket_fd) {
    std::lock_guard<std::mutex> handler_lock(handler_mutex);
    handler_map.erase(socket_fd);
}

void event::SocketEventMonitor::listen_for_event(int socket_fd, bool do_read, bool do_write) {
    {
        std::lock_guard<std::mutex> event_lock(event_mutex);
        event_queue.push({ socket_fd, do_read, do_write});
    }
    events_available.notify_one();
}

void event::SocketEventMonitor::set_event(SocketEvent& socket_event, pollfd& pollfd) {
    if (socket_event.do_read) {
        pollfd.events = pollfd.events | POLLIN;
    }
    if (socket_event.do_write) {
        pollfd.events = pollfd.events | POLLOUT;
    }
}

void event::SocketEventMonitor::populate_events(std::vector<pollfd>& pollfds) {
    std::unique_lock<std::mutex> event_lock(event_mutex);
    events_available.wait(event_lock, [&] { return !event_queue.empty(); });
    std::unordered_map<int , pollfd*> pollfd_map;
    while (!event_queue.empty()) {
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

void event::SocketEventMonitor::trigger_events(std::vector<pollfd>& pollfds) {
    std::lock_guard<std::mutex> handler_lock(handler_mutex);
    //std::cerr << "Trigger events: " << handler_map.size() << std::endl;
    for (auto it = pollfds.begin(); it != pollfds.end(); it++) {
        pollfd& pfd = *it;
        bool can_read = (pfd.revents & POLLIN) == POLLIN;
        bool can_write = (pfd.revents & POLLOUT) == POLLOUT;
        auto h_it = handler_map.find(pfd.fd);
        if (h_it == handler_map.end()) {
            std::cerr << "Cannot find handler: " << pfd.fd << std::endl;
            continue;
        }
        h_it->second->on_socket_event(can_read, can_write);
    } 
}

void event::SocketEventMonitor::start() {
    while (true) {
        std::vector<pollfd> pollfds;
        populate_events(pollfds);
        //std::cerr << "Poll fds size: " << pollfds.size() << std::endl;
        int fd_count = poll(pollfds.data(), pollfds.size(), 2000);
        if (fd_count == -1) {
            throw std::string("Poll Error: ") + std::string(strerror(errno));
        }
        //std::cerr << "Fd count: " << fd_count << std::endl;
        trigger_events(pollfds);
    }
}
