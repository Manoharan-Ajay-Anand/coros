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
#include <stdexcept>

coros::Socket::Socket(SocketDetails details, event::SocketEventMonitor& event_monitor, 
                      async::ThreadPool& thread_pool) 
        : details(details), event_monitor(event_monitor), thread_pool(thread_pool), 
          input_buffer(details.socket_fd), output_buffer(details.socket_fd) {
    this->marked_for_close = false;
    this->read_handler_set = false;
    this->write_handler_set = false;
    event_monitor.register_socket(details.socket_fd, *this);
}

void coros::Socket::listen_for_read(std::function<void()> handler) {
    {
        std::lock_guard<std::mutex> read_lock(read_mutex);
        if (read_handler_set) {
            throw std::runtime_error("Read handler already set");
        }
        read_handler_set = true;
        read_handler = handler;
    }
    event_monitor.listen_for_io(details.socket_fd);
}

void coros::Socket::listen_for_write(std::function<void()> handler) {
    {
        std::lock_guard<std::mutex> write_lock(write_mutex);
        if (write_handler_set) {
            throw std::runtime_error("Write handler already set");
        }
        write_handler_set = true;
        write_handler = handler;
    }
    event_monitor.listen_for_io(details.socket_fd);
}

void coros::Socket::on_socket_read(bool can_read) {
    std::function<void()> handler;
    {
        std::lock_guard<std::mutex> read_lock(read_mutex);
        if (!read_handler_set) {
            return;
        }
        if (!can_read) {
            return event_monitor.listen_for_io(details.socket_fd);
        }
        read_handler_set = false;
        handler = read_handler;
    }
    handler();
}

void coros::Socket::on_socket_write(bool can_write) {
    std::function<void()> handler;
    {
        std::lock_guard<std::mutex> write_lock(write_mutex);
        if (!write_handler_set) {
            return;
        }
        if (!can_write) {
            return event_monitor.listen_for_io(details.socket_fd);
        }
        write_handler_set = false;
        handler = write_handler;
    }
    handler();
}

void coros::Socket::on_socket_event(bool can_read, bool can_write) {
    if (marked_for_close) {
        return;
    }
    thread_pool.run([&, can_read]() {
        on_socket_read(can_read);
    });
    thread_pool.run([&, can_write]() {
        on_socket_write(can_write);
    });
}

coros::async::SocketReadAwaiter coros::Socket::read(uint8_t* dest, int size) {
    return { *this, input_buffer, dest, 0, size, std::runtime_error("") };
}

coros::async::SocketReadByteAwaiter coros::Socket::read_b() {
    return { *this, input_buffer, std::runtime_error("") };
}

coros::async::SocketWriteAwaiter coros::Socket::write(uint8_t* src, int size) {
    return { *this, output_buffer, src, 0, size, std::runtime_error("") };
}

coros::async::SocketWriteByteAwaiter coros::Socket::write_b(uint8_t b) {
    return { *this, output_buffer, b, std::runtime_error("") };
}

coros::async::SocketFlushAwaiter coros::Socket::flush() {
    return { *this, output_buffer, std::runtime_error("") };
}

int coros::Socket::get_fd() {
    return details.socket_fd;
}

void coros::Socket::close_socket() {
    if (marked_for_close.exchange(true)) {
        return;
    }
    close(details.socket_fd);
    input_buffer.close();
    output_buffer.close();
    event_monitor.deregister_socket(details.socket_fd);
    {
        std::lock_guard<std::mutex> read_lock(read_mutex);
        if (read_handler_set) {
            read_handler_set = false;
            read_handler();
        }
    }
    {
        std::lock_guard<std::mutex> write_lock(write_mutex);
        if (write_handler_set) {
            write_handler_set = false;
            write_handler();
        }
    }
}
