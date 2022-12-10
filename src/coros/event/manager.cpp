#include "manager.h"
#include "monitor.h"

coros::base::SocketEventManager::SocketEventManager(int socket_fd, 
                                                     SocketEventMonitor& event_monitor,
                                                     ThreadPool& thread_pool) 
        : event_monitor(event_monitor), read_executor(thread_pool), write_executor(thread_pool) {
    this->socket_fd = socket_fd;
    this->waiting_for_io = false;
    this->marked_for_close = false;
    event_monitor.register_socket(socket_fd, *this);
}

void coros::base::SocketEventManager::listen_for_io() {
    if (!waiting_for_io.exchange(true)) {
        event_monitor.listen_for_io(socket_fd);
    }
}

void coros::base::SocketEventManager::on_socket_event(bool can_read, bool can_write) {
    if (marked_for_close) {
        return;
    }
    waiting_for_io = false;
    bool missing_event = (!can_read && read_executor.has_handler()) || 
                         (!can_write && write_executor.has_handler());
    if (can_read) {
        read_executor.on_event();
    }
    if (can_write) {
        write_executor.on_event();
    }
    if (missing_event) {
        listen_for_io();
    }
}

void coros::base::SocketEventManager::set_read_handler(std::function<void()> handler) {
    read_executor.set_handler(handler);
    listen_for_io();
}

void coros::base::SocketEventManager::set_write_handler(std::function<void()> handler) {
    write_executor.set_handler(handler);
    listen_for_io();
}

void coros::base::SocketEventManager::close() {
    if (marked_for_close.exchange(true)) {
        return;
    }
    event_monitor.deregister_socket(socket_fd);
    read_executor.on_event();
    write_executor.on_event();
}
