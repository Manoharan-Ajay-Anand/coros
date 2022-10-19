#include "socket_buffer.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <cerrno>
#include <stdexcept>

coros::memory::SocketBuffer::SocketBuffer(int socket_fd, async::ThreadPool& thread_pool, 
                                          Socket& socket) 
        : thread_pool(thread_pool), socket(socket) {
    this->socket_fd = socket_fd;
    this->read_index = 0;
    this->write_index = 0;
    this->is_closed = false;
}

int coros::memory::SocketBuffer::get_position(int index) {
    return index % BUFFER_LENGTH;
}

void coros::memory::SocketBuffer::reset_read_write_indices() {
    if (read_index > BUFFER_LENGTH && write_index > BUFFER_LENGTH) {
        read_index -= BUFFER_LENGTH;
        write_index -= BUFFER_LENGTH;
    }
}

int coros::memory::SocketBuffer::get_fd() {
    return socket_fd;
}

int coros::memory::SocketBuffer::remaining() {
    return write_index - read_index;
}

int coros::memory::SocketBuffer::capacity() {
    return BUFFER_LENGTH - remaining();
}
