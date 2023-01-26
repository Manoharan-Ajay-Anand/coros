#include "manager.h"
#include "monitor.h"

#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

coros::base::SocketEventManager::SocketEventManager(SocketEventMonitor& event_monitor,
                                                    ThreadPool& thread_pool) 
        : event_monitor(event_monitor), read_executor(thread_pool), write_executor(thread_pool) {
    this->is_registered = false;
    this->waiting_for_io = false;
    this->marked_for_close = false;
}

void coros::base::SocketEventManager::register_socket_fd(int socket_fd) {
    {
        std::lock_guard<std::mutex> guard(manager_mutex);
        if (is_registered) {
            throw std::runtime_error("SocketEventManager register_socket: already registered");
        }
        this->socket_fd = socket_fd;
        is_registered = true;
    }
    event_monitor.register_socket(socket_fd, *this);
}

void coros::base::SocketEventManager::listen_for_io() {
    if (!waiting_for_io.exchange(true)) {
        event_monitor.listen_for_io(socket_fd);
    }
}

void coros::base::SocketEventManager::on_socket_event(bool can_read, bool can_write) {
    bool missing_event = false;
    {
        std::lock_guard<std::mutex> guard(manager_mutex);
        if (marked_for_close) {
            return;
        }
        waiting_for_io = false;
        missing_event = (!can_read && read_executor.has_handler()) || 
                            (!can_write && write_executor.has_handler());
        if (can_read) {
            read_executor.on_event();
        }
        if (can_write) {
            write_executor.on_event();
        }
    }
    if (missing_event) {
        listen_for_io();
    }
}

void coros::base::SocketEventManager::set_read_handler(std::function<void()> handler) {
    {
        std::lock_guard<std::mutex> guard(manager_mutex);
        if (!is_registered) {
            throw std::runtime_error("SocketEventManager set_read_handler: not registered");
        }
        read_executor.set_handler(handler);
    }
    listen_for_io();
}

void coros::base::SocketEventManager::set_write_handler(std::function<void()> handler) {
    {
        std::lock_guard<std::mutex> guard(manager_mutex);
        if (!is_registered) {
            throw std::runtime_error("SocketEventManager set_write_handler: not registered");
        }
        write_executor.set_handler(handler);
    }
    listen_for_io();
}

void coros::base::SocketEventManager::close() {
    {
        std::lock_guard<std::mutex> guard(manager_mutex);
        if (marked_for_close) {
            return;
        }
        marked_for_close = true;
        read_executor.on_event();
        write_executor.on_event();
    }
    event_monitor.deregister_socket(socket_fd);
}
