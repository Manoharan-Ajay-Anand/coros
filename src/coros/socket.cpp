#include "socket.h"
#include "server.h"
#include "async/thread_pool.h"
#include "event/event.h"
#include "async/read_awaiter.h"
#include "async/write_awaiter.h"
#include "memory/read_socket_buffer.h"
#include "memory/write_socket_buffer.h"

#include <cstdint>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <mutex>
#include <stdexcept>

coros::Socket::Socket(int socket_fd, sockaddr_storage client_addr, socklen_t addr_size, 
    Server& server, event::SocketEventMonitor& event_monitor, async::ThreadPool& thread_pool) 
: socket_fd(socket_fd), server(server), event_monitor(event_monitor), thread_pool(thread_pool), 
  input_buffer(socket_fd), output_buffer(socket_fd) {
    this->client_addr = client_addr;
    this->addr_size = addr_size;
    this->marked_for_close = false;
    this->read_handler_set = false;
    this->write_handler_set = false;
}

coros::Socket::~Socket() {
    marked_for_close = true;
    event_monitor.deregister_socket(socket_fd);
    close(socket_fd);
    if (read_handler_set) {
        cleanup_read();
    }
    if (write_handler_set) {
        cleanup_write();
    }
}

void coros::Socket::listen_for_read(std::function<void()> handler, std::function<void()> cleanup) {
    std::lock_guard<std::mutex> read_lock(read_handler_mutex);
    if (read_handler_set) {
        throw std::runtime_error("Read handler already set");
    }
    read_handler_set = true;
    read_handler = handler;
    cleanup_read = cleanup;
    event_monitor.listen_for_read(socket_fd);
}

void coros::Socket::listen_for_write(std::function<void()> handler, std::function<void()> cleanup) {
    std::lock_guard<std::mutex> write_lock(write_handler_mutex);
    if (write_handler_set) {
        throw std::runtime_error("Write handler already set");
    }
    write_handler_set = true;
    write_handler = handler;
    cleanup_write = cleanup;
    event_monitor.listen_for_write(socket_fd);
}

void coros::Socket::on_socket_read(bool can_read) {
    std::lock_guard<std::mutex> read_lock(read_handler_mutex);
    if (!read_handler_set) {
        return;
    }
    if (!can_read) {
        return event_monitor.listen_for_read(socket_fd);
    }
    read_handler_set = false;
    thread_pool.run(read_handler);
}

void coros::Socket::on_socket_write(bool can_write) {
    std::lock_guard<std::mutex> write_lock(write_handler_mutex);
    if (!write_handler_set) {
        return;
    }
    if (!can_write) {
        return event_monitor.listen_for_write(socket_fd);
    }
    write_handler_set = false;
    thread_pool.run(write_handler);
}

void coros::Socket::on_socket_event(bool can_read, bool can_write) {
    if (marked_for_close) {
        return;
    }
    on_socket_read(can_read);
    on_socket_write(can_write);
}

coros::async::SocketReadAwaiter coros::Socket::read(uint8_t* dest, int size) {
    return { *this, input_buffer, dest, 0, size, std::runtime_error("") };
}

coros::async::SocketWriteAwaiter coros::Socket::write(uint8_t* src, int size) {
    return { *this, output_buffer, src, 0, size, std::runtime_error("") };
}

coros::async::SocketFlushAwaiter coros::Socket::flush() {
    return { *this, output_buffer, std::runtime_error("") };
}

void coros::Socket::close_socket() {
    server.destroy_socket(socket_fd);
}
