#include "socket.h"
#include "server.h"
#include "coros/async/thread_pool.h"
#include "coros/event/manager.h"
#include "coros/async/read_awaiter.h"
#include "coros/async/write_awaiter.h"

#include <cstdint>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>

#define SOCKET_BUFFER_SIZE 8192

coros::network::Socket::Socket(SocketDetails details, event::SocketEventMonitor& event_monitor, 
                      async::ThreadPool& thread_pool) 
        : details(details), event_manager(details.socket_fd, event_monitor, thread_pool), 
          stream(details.socket_fd),
          input_buffer(SOCKET_BUFFER_SIZE), 
          output_buffer(SOCKET_BUFFER_SIZE) {
}

coros::async::SocketReadAwaiter coros::network::Socket::read(std::byte* dest, int size) {
    return { stream, event_manager, input_buffer, dest, 0, size, std::runtime_error("") };
}

coros::async::SocketReadByteAwaiter coros::network::Socket::read_b() {
    return { stream, event_manager, input_buffer, std::runtime_error("") };
}

coros::async::SocketWriteAwaiter coros::network::Socket::write(std::byte* src, int size) {
    return { stream, event_manager, output_buffer, src, 0, size, std::runtime_error("") };
}

coros::async::SocketWriteByteAwaiter coros::network::Socket::write_b(const std::byte b) {
    return { stream, event_manager, output_buffer, b, std::runtime_error("") };
}

coros::async::SocketFlushAwaiter coros::network::Socket::flush() {
    return { stream, event_manager, output_buffer, std::runtime_error("") };
}

int coros::network::Socket::get_fd() {
    return details.socket_fd;
}

void coros::network::Socket::close_socket() {
    stream.close();
    event_manager.close();
    close(details.socket_fd);
}
