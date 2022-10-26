#include "read_socket_buffer.h"
#include "coros/socket.h"
#include "coros/async/thread_pool.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <cerrno>
#include <stdexcept>
#include <optional>
#include <mutex>
#include <functional>
#include <algorithm>

coros::memory::SocketReadBuffer::SocketReadBuffer(int socket_fd) : SocketBuffer(socket_fd) {
}

uint8_t coros::memory::SocketReadBuffer::read_b() {
    if (is_closed) {
        throw std::runtime_error("SocketBuffer read error: Socket already closed");
    }
    if (remaining() == 0) {
        throw std::runtime_error("SocketBuffer read_b error: Read size more than remaining");
    }
    uint8_t b = buffer[get_position(read_index)];
    ++read_index;
    reset_read_write_indices();
    return b;
}

void coros::memory::SocketReadBuffer::read(uint8_t* dest, int size) {
    if (is_closed) {
        throw std::runtime_error("SocketBuffer read error: Socket already closed");
    }
    if (size > remaining()) {
        throw std::runtime_error("SocketBuffer read error: Read size more than remaining");
    }
    while (size > 0) {
        int read_position = get_position(read_index);
        int size_to_read = std::min(size, BUFFER_LENGTH - read_position);
        std::memcpy(dest, buffer.data() + read_position, size_to_read);
        size -= size_to_read;
        dest += size_to_read;
        read_index += size_to_read;
    }
    reset_read_write_indices();
}

int coros::memory::SocketReadBuffer::recv_size(int size) {
    while (size > 0) {
        if (is_closed) {
            throw std::runtime_error("SocketBuffer read error: Socket already closed");
        }
        int size_read = recv(socket_fd, buffer.data() + get_position(write_index), size, 0);
        if (size_read == 0) {
            throw std::runtime_error("SocketBuffer recv_socket failed: client disconnected");
        }
        if (size_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return SOCKET_OP_WOULD_BLOCK;
            }
            throw std::runtime_error(std::string("SocketBuffer recv(): ").append(strerror(errno)));
        }
        write_index += size_read;
        size -= size_read;
    }
    return SOCKET_OP_CONTINUE;
}

int coros::memory::SocketReadBuffer::recv_socket() {
    while (capacity() > 0) {
        int status = recv_size(std::min(capacity(), BUFFER_LENGTH - get_position(write_index)));
        if (status == SOCKET_OP_WOULD_BLOCK) {
            return status;
        }
    }
    return SOCKET_OP_CONTINUE;
}
