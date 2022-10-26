#include "socket.h"
#include "server.h"
#include "async/thread_pool.h"
#include "event/manager.h"
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
        : details(details), event_manager(details.socket_fd, event_monitor, thread_pool), 
          input_buffer(details.socket_fd), 
          output_buffer(details.socket_fd) {
}

coros::async::SocketReadAwaiter coros::Socket::read(uint8_t* dest, int size) {
    return { event_manager, input_buffer, dest, 0, size, std::runtime_error("") };
}

coros::async::SocketReadByteAwaiter coros::Socket::read_b() {
    return { event_manager, input_buffer, std::runtime_error("") };
}

coros::async::SocketWriteAwaiter coros::Socket::write(const uint8_t* src, int size) {
    return { event_manager, output_buffer, src, 0, size, std::runtime_error("") };
}

coros::async::SocketWriteByteAwaiter coros::Socket::write_b(const uint8_t b) {
    return { event_manager, output_buffer, b, std::runtime_error("") };
}

coros::async::SocketFlushAwaiter coros::Socket::flush() {
    return { event_manager, output_buffer, std::runtime_error("") };
}

int coros::Socket::get_fd() {
    return details.socket_fd;
}

void coros::Socket::close_socket() {
    input_buffer.close();
    output_buffer.close();
    event_manager.close();
    close(details.socket_fd);
}
