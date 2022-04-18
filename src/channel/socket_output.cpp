#include "socket_output.h"
#include "server/socket.h"
#include "memory/socket_buffer.h"
#include "concurrent/thread_pool.h"
#include "concurrent/coroutine.h"

#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <coroutine>

channel::SocketOutputChannel::SocketOutputChannel(server::Socket& socket) : socket(socket) {
}

void channel::SocketOutputChannel::write_buffer_from_mem(std::coroutine_handle<> handle, 
    uint8_t* src, int offset, int size) {
    while (size > 0) {
        if (buffer.capacity() > 0) {
            int size_to_write = std::min(buffer.capacity(), size);
            buffer.write(src + offset, size_to_write);
            offset += size_to_write;
            size -= size_to_write;
            continue;
        }
        int status = buffer.send_socket(socket.get_fd());
        if (status == SOCKET_OP_WOULD_BLOCK) {
            return socket.set_write_handler([&, handle, src, offset, size]() {
                write_buffer_from_mem(handle, src, offset, size);
            });
        }
    }
    handle.resume();
}

concurrent::Awaiter channel::SocketOutputChannel::write_from_mem(uint8_t* src, int size) {
    return concurrent::Awaiter {
        [&, src, size](std::coroutine_handle<> handle) { 
            write_buffer_from_mem(handle, src, 0, size); 
        }
    };
}

void channel::SocketOutputChannel::flush_buffer(std::coroutine_handle<> handle) {
    while (buffer.remaining() > 0) {
        int status = buffer.send_socket(socket.get_fd());
        if (status == SOCKET_OP_WOULD_BLOCK) {
            return socket.set_write_handler([&, handle]() {
                flush_buffer(handle);
            });
        }
    }
    handle.resume();
}

concurrent::Awaiter channel::SocketOutputChannel::flush() {
    return concurrent::Awaiter {
        [&](std::coroutine_handle<> handle) {
            flush_buffer(handle); 
        }
    };
}
