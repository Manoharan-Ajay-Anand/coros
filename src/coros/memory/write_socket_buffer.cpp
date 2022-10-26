#include "write_socket_buffer.h"
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

coros::memory::SocketWriteBuffer::SocketWriteBuffer(int socket_fd) : SocketBuffer(socket_fd) {
}

void coros::memory::SocketWriteBuffer::write_b(const uint8_t b) {
    if (is_closed) {
        throw std::runtime_error("SocketBuffer write error: Socket already closed");
    }
    if (capacity() == 0) {
        throw std::runtime_error("SocketBuffer write error: Write size more than capacity");
    }
    buffer[get_position(write_index)] = b;
    ++write_index;
    reset_read_write_indices();
}

void coros::memory::SocketWriteBuffer::write(const uint8_t* src, int size) {
    if (is_closed) {
        throw std::runtime_error("SocketBuffer write error: Socket already closed");
    }
    if (size > capacity()) {
        throw std::runtime_error("SocketBuffer write error: Write size more than capacity");
    }
    while (size > 0) {
        int write_position = get_position(write_index);
        int size_to_write = std::min(size, BUFFER_LENGTH - write_position);
        std::memcpy(buffer.data() + write_position, src, size_to_write);
        size -= size_to_write;
        src += size_to_write;
        write_index += size_to_write;
    }
    reset_read_write_indices();
}

int coros::memory::SocketWriteBuffer::send_size(int size) {
    while (size > 0) {
        if (is_closed) {
            throw std::runtime_error("SocketBuffer write error: Socket already closed");
        }
        int size_written = send(socket_fd, buffer.data() + get_position(read_index), size, 0);
        if (size_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return SOCKET_OP_WOULD_BLOCK;
            }
            throw std::runtime_error(std::string("SocketBuffer send(): ").append(strerror(errno)));
        }
        read_index += size_written;
        size -= size_written;
    }
    return SOCKET_OP_CONTINUE;
}

int coros::memory::SocketWriteBuffer::send_socket() {
    while (remaining() > 0) {
        int status = send_size(std::min(remaining(), BUFFER_LENGTH - get_position(read_index)));
        if (status == SOCKET_OP_WOULD_BLOCK) {
            return status;
        }
    }
    return SOCKET_OP_CONTINUE;
}
