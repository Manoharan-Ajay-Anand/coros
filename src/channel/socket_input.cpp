#include "socket_input.h"
#include "server/socket.h"
#include "memory/socket_buffer.h"
#include "concurrent/thread_pool.h"
#include "concurrent/coroutine.h"

#include <coroutine>
#include <algorithm>
#include <iostream>

channel::SocketInputChannel::SocketInputChannel(server::Socket& socket) : socket(socket) {
}

void channel::SocketInputChannel::read_buffer_to_mem(std::coroutine_handle<> handle, 
    uint8_t* dest, int offset, int size) {
    while (size > 0) {
        if (buffer.remaining() > 0) {
            int size_to_read = std::min(buffer.remaining(), size);
            buffer.read(dest + offset, size_to_read);
            offset += size_to_read;
            size -= size_to_read;
            continue;
        }
        int status = buffer.recv_socket(socket.get_fd());
        if (status == SOCKET_OP_WOULD_BLOCK) {
            return socket.set_read_handler([&, handle, dest, offset, size]() {
                read_buffer_to_mem(handle, dest, offset, size);
            });
        }
    }
    handle.resume();
}

concurrent::Awaiter channel::SocketInputChannel::read_to_mem(uint8_t* dest, int size) {
    return concurrent::Awaiter {
        [&, dest, size](std::coroutine_handle<> handle) { 
            read_buffer_to_mem(handle, dest, 0, size); 
        }
    };
}
