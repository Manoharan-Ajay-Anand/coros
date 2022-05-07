#include "awaiter.h"
#include "socket.h"
#include "memory/read_socket_buffer.h"
#include "memory/write_socket_buffer.h"

#include <algorithm>
#include <coroutine>
#include <iostream>
#include <stdexcept>

void server::SocketReadAwaiter::read(std::coroutine_handle<> handle) {
    try {
        while (size > 0) {
            if (buffer.remaining() > 0) {
                int size_to_read = std::min(buffer.remaining(), size);
                buffer.read(dest + offset, size_to_read);
                offset += size_to_read;
                size -= size_to_read;
                continue;
            }
            int status = buffer.recv_socket();
            if (status == SOCKET_OP_WOULD_BLOCK) {
                return socket.listen_for_read([&, handle]() {
                    read(handle);
                });
            }
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

bool server::SocketReadAwaiter::await_ready() noexcept {
    int sizeToRead = std::min(size, buffer.remaining());
    buffer.read(dest + offset, sizeToRead);
    offset += sizeToRead;
    size -= sizeToRead;
    return size == 0;
}

void server::SocketReadAwaiter::await_suspend(std::coroutine_handle<> handle) {
    read(handle);
}

void server::SocketReadAwaiter::await_resume() {
    if (size > 0) {
        throw error;
    }
}

void server::SocketWriteAwaiter::write(std::coroutine_handle<> handle) {
    try {
        while (size > 0) {
            if (buffer.capacity() > 0) {
                int size_to_write = std::min(buffer.capacity(), size);
                buffer.write(src + offset, size_to_write);
                offset += size_to_write;
                size -= size_to_write;
                continue;
            }
            int status = buffer.send_socket();
            if (status == SOCKET_OP_WOULD_BLOCK) {
                return socket.listen_for_write([&, handle]() {
                    write(handle);
                });
            }
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

bool server::SocketWriteAwaiter::await_ready() noexcept {
    int size_to_write = std::min(buffer.capacity(), size);
    buffer.write(src + offset, size_to_write);
    offset += size_to_write;
    size -= size_to_write;
    return size == 0;
}

void server::SocketWriteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    write(handle);
}

void server::SocketWriteAwaiter::await_resume() {
    if (size > 0) {
        throw error;
    }
}

void server::SocketFlushAwaiter::flush(std::coroutine_handle<> handle) {
    try {
        while (buffer.remaining() > 0) {
            int status = buffer.send_socket();
            if (status == SOCKET_OP_WOULD_BLOCK) {
                return socket.listen_for_write([&, handle]() {
                    flush(handle);
                });
            }
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

bool server::SocketFlushAwaiter::await_ready() noexcept {
    return buffer.remaining() == 0;
}

void server::SocketFlushAwaiter::await_suspend(std::coroutine_handle<> handle) {
    flush(handle);
}

void server::SocketFlushAwaiter::await_resume() {
    if (buffer.remaining() > 0) {
        throw error;
    }
}
