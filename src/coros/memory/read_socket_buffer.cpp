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

coros::memory::SocketReadBuffer::SocketReadBuffer(int socket_fd, async::ThreadPool& thread_pool,
                                                  Socket& socket) 
        : SocketBuffer(socket_fd, thread_pool, socket) {
}

void coros::memory::SocketReadBuffer::read(uint8_t* dest, int size) {
    if (is_closed) {
        throw std::runtime_error("SocketBuffer read error: Socket already closed");
    }
    if (size > remaining()) {
        throw std::runtime_error("SocketBuffer read error: Read size more than remaining");
    }
    std::memcpy(dest, buffer.data() + start, size);
    start += size;
}

uint8_t coros::memory::SocketReadBuffer::read_b() {
    if (is_closed) {
        throw std::runtime_error("SocketBuffer read error: Socket already closed");
    }
    if (remaining() == 0) {
        throw std::runtime_error("SocketBuffer read_b error: Read size more than remaining");
    }
    uint8_t b = buffer[start];
    ++start;
    return b;
}

int coros::memory::SocketReadBuffer::recv_socket() {
    compact();
    while (end < BUFFER_LIMIT) {
        if (is_closed) {
            throw std::runtime_error("SocketBuffer read error: Socket already closed");
        }
        int size_read = recv(socket_fd, buffer.data() + end, capacity(), 0);
        if (size_read == 0) {
            throw std::runtime_error("SocketBuffer recv_socket failed: client disconnected");
        }
        if (size_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return SOCKET_OP_WOULD_BLOCK;
            }
            throw std::runtime_error(std::string("SocketBuffer recv(): ").append(strerror(errno)));
        }
        end += size_read;
    }
    return SOCKET_OP_CONTINUE;
}

void coros::memory::SocketReadBuffer::set_read_handler(std::function<void()> handler) {
    if (is_closed) {
        throw std::runtime_error("SocketBuffer read error: Socket already closed");
    }
    std::lock_guard<std::mutex> guard(read_mutex);
    if (read_handler) {
        throw std::runtime_error("SocketBuffer read error: Read Handler already set");
    }
    read_handler = handler;
    socket.listen_for_io();
}

void coros::memory::SocketReadBuffer::on_read(bool can_read) {
    std::lock_guard<std::mutex> guard(read_mutex);
    if (!read_handler) {
        return;
    }
    if (!can_read) {
        return socket.listen_for_io();
    }
    thread_pool.run(read_handler.value());
    read_handler.reset();
}

void coros::memory::SocketReadBuffer::close() {
    if (is_closed.exchange(true)) {
        return;
    }
    if (read_handler) {
        thread_pool.run(read_handler.value());
    }
}
