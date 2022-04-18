#include "socket_buffer.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <cerrno>
#include <stdexcept>

memory::SocketBuffer::SocketBuffer() {
    this->data = new uint8_t[BUFFER_LIMIT];
    this->start = 0;
    this->end = 0;
    this->limit = BUFFER_LIMIT;
}

memory::SocketBuffer::~SocketBuffer() {
    delete[] data; 
}

void memory::SocketBuffer::compact() {
    if (start == 0) {
        return;
    }
    int dest = 0;
    int src = start;
    while (src < end) {
        data[dest] = data[src];
        dest++;
        src++;
    }
    start = 0;
    end = dest;
}

int memory::SocketBuffer::remaining() {
    return end - start;
}

void memory::SocketBuffer::read(uint8_t* dest, int size) {
    if (size > remaining()) {
        throw std::runtime_error("SocketBuffer read error: Read size more than remaining");
    }
    std::memcpy(dest, data + start, size);
    start += size;
}

int memory::SocketBuffer::recv_socket(int socket_fd) {
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

int memory::SocketBuffer::capacity() {
    return limit - end;
}

void memory::SocketBuffer::write(uint8_t* src, int size) {
    compact();
    if (size > capacity()) {
        throw std::runtime_error("SocketBuffer write error: Write size more than capacity");
    }
    std::memcpy(data + end, src, size);
    end += size;
}

int memory::SocketBuffer::send_socket(int socket_fd) {
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
