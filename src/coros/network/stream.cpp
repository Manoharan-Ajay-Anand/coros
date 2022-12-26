#include "stream.h"
#include "coros/memory/buffer.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

coros::base::SocketStream::SocketStream(int socket_fd) {
    this->socket_fd = socket_fd;
    this->is_closed = false;
}

coros::base::SocketOperation coros::base::SocketStream::recv_from_socket(ByteBuffer& buffer) {
    if (is_closed) {
        return SOCKET_OP_CLOSE;
    }
    while (buffer.get_total_capacity() > 0) {
        coros::base::IOChunk chunk = buffer.get_write_chunk();
        int size_read = recv(socket_fd, chunk.data, chunk.size, 0);
        if (size_read == 0) {
            this->close();
            return SOCKET_OP_CLOSE;
        }
        if (size_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return SOCKET_OP_BLOCK;
            }
            throw std::runtime_error(std::string("Network recv(): ").append(strerror(errno)));
        }
        buffer.increment_write_pointer(size_read);
    }
    return SOCKET_OP_SUCCESS;
}

coros::base::SocketOperation coros::base::SocketStream::send_to_socket(ByteBuffer& buffer) {
    if (is_closed) {
        return SOCKET_OP_CLOSE;
    }
    while (buffer.get_total_remaining() > 0) {
        coros::base::IOChunk chunk = buffer.get_read_chunk();
        int size_written = send(socket_fd, chunk.data, chunk.size, 0);
        if (size_written == 0) {
            this->close();
            return SOCKET_OP_CLOSE;
        }
        if (size_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return SOCKET_OP_BLOCK;
            }
            throw std::runtime_error(std::string("Network send(): ").append(strerror(errno)));
        }
        buffer.increment_read_pointer(size_written);
    }
    return SOCKET_OP_SUCCESS;
}

void coros::base::SocketStream::close() {
    is_closed = true;
}
