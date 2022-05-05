#include "write_socket_buffer.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <cerrno>
#include <stdexcept>

memory::SocketWriteBuffer::SocketWriteBuffer(int socket_fd) : SocketBuffer(socket_fd) {
}

void memory::SocketWriteBuffer::write(uint8_t* src, int size) {
    compact();
    if (size > capacity()) {
        throw std::runtime_error("SocketBuffer write error: Write size more than capacity");
    }
    std::memcpy(data + end, src, size);
    end += size;
}

int memory::SocketWriteBuffer::send_socket() {
    int prev_start = start;
    while (start < end) {
        int size_written = send(socket_fd, data + start, remaining(), 0);
        if (size_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            throw std::runtime_error(std::string("SocketBuffer send(): ").append(strerror(errno)));
        }
        start += size_written;
    }
    if (start > prev_start) {
        return SOCKET_OP_CONTINUE;
    }
    return SOCKET_OP_WOULD_BLOCK;
}
