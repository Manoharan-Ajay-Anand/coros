#include "socket_buffer.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <cerrno>
#include <stdexcept>

memory::SocketBuffer::SocketBuffer(int socket_fd) {
    this->socket_fd = socket_fd;
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

int memory::SocketBuffer::get_fd() {
    return socket_fd;
}

int memory::SocketBuffer::remaining() {
    return end - start;
}

int memory::SocketBuffer::capacity() {
    return limit - end;
}
