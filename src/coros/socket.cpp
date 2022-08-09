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
        : details(details), event_monitor(event_monitor), 
          input_buffer(details.socket_fd, thread_pool, *this), 
          output_buffer(details.socket_fd, thread_pool, *this) {
    this->waiting_for_io = false;
    this->marked_for_close = false;
    event_monitor.register_socket(details.socket_fd, *this);
}

void coros::Socket::listen_for_io() {
    if (!waiting_for_io.exchange(true)) {
        event_monitor.listen_for_io(details.socket_fd);
    }
}

void coros::Socket::on_socket_event(bool can_read, bool can_write) {
    if (marked_for_close) {
        return;
    }
    waiting_for_io = false;
    input_buffer.on_read(can_read);
    output_buffer.on_write(can_write);
}

coros::async::SocketReadAwaiter coros::Socket::read(uint8_t* dest, int size) {
    return { input_buffer, dest, 0, size, std::runtime_error("") };
}

coros::async::SocketReadByteAwaiter coros::Socket::read_b() {
    return { input_buffer, std::runtime_error("") };
}

coros::async::SocketWriteAwaiter coros::Socket::write(const uint8_t* src, int size) {
    return { output_buffer, src, 0, size, std::runtime_error("") };
}

coros::async::SocketWriteByteAwaiter coros::Socket::write_b(const uint8_t b) {
    return { output_buffer, b, std::runtime_error("") };
}

coros::async::SocketFlushAwaiter coros::Socket::flush() {
    return { output_buffer, std::runtime_error("") };
}

int coros::Socket::get_fd() {
    return details.socket_fd;
}

void coros::Socket::close_socket() {
    if (marked_for_close.exchange(true)) {
        return;
    }
    input_buffer.close();
    output_buffer.close();
    event_monitor.deregister_socket(details.socket_fd);
    close(details.socket_fd);
}
