#include "write_socket_buffer.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <cerrno>
#include <stdexcept>

coros::memory::SocketWriteBuffer::SocketWriteBuffer(int socket_fd) : SocketBuffer(socket_fd) {
}

void coros::memory::SocketWriteBuffer::write(uint8_t* src, int size) {
    if (is_closed) {
        throw std::runtime_error("SocketBuffer write error: Socket already closed");
    }
    compact();
    if (size > capacity()) {
        throw std::runtime_error("SocketBuffer write error: Write size more than capacity");
    }
    std::memcpy(buffer.data() + end, src, size);
    end += size;
}

void coros::memory::SocketWriteBuffer::write_b(uint8_t b) {
    if (is_closed) {
        throw std::runtime_error("SocketBuffer write error: Socket already closed");
    }
    if (capacity() == 0) {
        throw std::runtime_error("SocketBuffer write error: Write size more than capacity");
    }
    buffer[end] = b;
    end++;
}

int coros::memory::SocketWriteBuffer::send_socket() {
    while (start < end) {
        if (is_closed) {
            throw std::runtime_error("SocketBuffer write error: Socket already closed");
        }
        int size_written = send(socket_fd, buffer.data() + start, remaining(), 0);
        if (size_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return SOCKET_OP_WOULD_BLOCK;
            }
            throw std::runtime_error(std::string("SocketBuffer send(): ").append(strerror(errno)));
        }
        start += size_written;
    }
    return SOCKET_OP_CONTINUE;
}
