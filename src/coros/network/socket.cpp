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

coros::base::Socket::Socket(SocketDetails details, SocketEventMonitor& event_monitor, 
                            ThreadPool& thread_pool) 
        : details(details), event_manager(details.socket_fd, event_monitor, thread_pool), 
          stream(details.socket_fd), input_buffer(SOCKET_BUFFER_SIZE), 
          output_buffer(SOCKET_BUFFER_SIZE) {
}

coros::base::SocketReadAwaiter coros::base::Socket::read(std::byte* dest, int size) {
    return { stream, event_manager, input_buffer, dest, 0, size, std::runtime_error("") };
}

coros::base::SocketReadByteAwaiter coros::base::Socket::read_b() {
    return { stream, event_manager, input_buffer, std::runtime_error("") };
}

coros::base::SocketWriteAwaiter coros::base::Socket::write(std::byte* src, int size) {
    return { stream, event_manager, output_buffer, src, 0, size, std::runtime_error("") };
}

coros::base::SocketWriteByteAwaiter coros::base::Socket::write_b(const std::byte b) {
    return { stream, event_manager, output_buffer, b, std::runtime_error("") };
}

coros::base::SocketFlushAwaiter coros::base::Socket::flush() {
    return { stream, event_manager, output_buffer, std::runtime_error("") };
}

int coros::base::Socket::get_fd() {
    return details.socket_fd;
}

void coros::base::Socket::close_socket() {
    stream.close();
    event_manager.close();
    close(details.socket_fd);
}
