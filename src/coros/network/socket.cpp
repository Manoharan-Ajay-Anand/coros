#include "socket.h"
#include "server.h"
#include "coros/async/thread_pool.h"
#include "coros/event/manager.h"
#include "coros/awaiter/flush_awaiter.h"
#include "coros/awaiter/read_awaiter.h"
#include "coros/awaiter/skip_awaiter.h"
#include "coros/awaiter/write_awaiter.h"

#include <atomic>
#include <cstddef>
#include <sys/socket.h>
#include <unistd.h>

#define SOCKET_BUFFER_SIZE 8192

coros::base::Socket::Socket(SocketDetails details, SocketEventMonitor& event_monitor, 
                            ThreadPool& thread_pool) 
        : details(details), event_manager(event_monitor, thread_pool), 
          stream(details.socket_fd), input_buffer(SOCKET_BUFFER_SIZE), 
          output_buffer(SOCKET_BUFFER_SIZE) {
    is_closed = false;
    event_manager.register_socket_fd(details.socket_fd);
}

coros::base::SocketReadAwaiter coros::base::Socket::read(std::byte* dest, long long size) {
    return { stream, event_manager, input_buffer, dest, size };
}

coros::base::SocketReadByteAwaiter coros::base::Socket::read_b() {
    return { stream, event_manager, input_buffer };
}

coros::base::SocketSkipAwaiter coros::base::Socket::skip(long long size) {
    return { stream, event_manager, input_buffer, size };
}

coros::base::SocketWriteAwaiter coros::base::Socket::write(std::byte* src, long long size) {
    return { stream, event_manager, output_buffer, src, size };
}

coros::base::SocketWriteByteAwaiter coros::base::Socket::write_b(const std::byte b) {
    return { stream, event_manager, output_buffer, b };
}

coros::base::SocketFlushAwaiter coros::base::Socket::flush() {
    return { stream, event_manager, output_buffer };
}

int coros::base::Socket::get_fd() {
    return details.socket_fd;
}

void coros::base::Socket::close_socket() {
    if (is_closed.exchange(true)) {
        return;
    }
    stream.close();
    event_manager.close();
    close(details.socket_fd);
}
