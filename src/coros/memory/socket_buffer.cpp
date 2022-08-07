#include "socket_buffer.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <cerrno>
#include <stdexcept>

coros::memory::SocketBuffer::SocketBuffer(int socket_fd) {
    this->socket_fd = socket_fd;
    this->start = 0;
    this->end = 0;
    this->is_closed = false;
}

void coros::memory::SocketBuffer::compact() {
    if (start == 0) {
        return;
    }
    int dest = 0;
    int src = start;
    while (src < end) {
        buffer[dest] = buffer[src];
        ++dest;
        ++src;
    }
    start = 0;
    end = dest;
}

int coros::memory::SocketBuffer::get_fd() {
    return socket_fd;
}

int coros::memory::SocketBuffer::remaining() {
    return end - start;
}

int coros::memory::SocketBuffer::capacity() {
    return BUFFER_LIMIT - end;
}

void coros::memory::SocketBuffer::close() {
    is_closed = true;
}
