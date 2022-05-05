#include "read_socket_buffer.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <cerrno>
#include <stdexcept>

memory::SocketReadBuffer::SocketReadBuffer(int socket_fd) : SocketBuffer(socket_fd) {
}

void memory::SocketReadBuffer::read(uint8_t* dest, int size) {
    if (size > remaining()) {
        throw std::runtime_error("SocketBuffer read error: Read size more than remaining");
    }
    std::memcpy(dest, data + start, size);
    start += size;
}

int memory::SocketReadBuffer::recv_socket() {
    compact();
    int prev_end = end;
    while (end < limit) {
        int size_read = recv(socket_fd, data + end, capacity(), 0);
        if (size_read == 0) {
            throw std::runtime_error("SocketBuffer recv_socket failed: client disconnected");
        }
        if (size_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            throw std::runtime_error(std::string("SocketBuffer recv(): ").append(strerror(errno)));
        }
        end += size_read;
    }
    if (end > prev_end) {
        return SOCKET_OP_CONTINUE;
    }
    return SOCKET_OP_WOULD_BLOCK;
}
