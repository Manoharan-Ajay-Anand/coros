#include "read_awaiter.h"
#include "coros/event/manager.h"
#include "coros/memory/buffer.h"
#include "coros/network/stream.h"

#include <algorithm>
#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <cstddef>

void coros::base::SocketReadAwaiter::read(std::coroutine_handle<> handle) {
    try {
        while (size > 0) {
            int status = stream.recv_from_socket(buffer);
            read_available();
            if (status == SOCKET_OP_BLOCK && size > 0) {
                return event_manager.set_read_handler([&, handle]() {
                    read(handle);
                });
            }
            if (status == SOCKET_OP_CLOSE && size > 0) {
                throw std::runtime_error("Socket has been closed");
            }
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

void coros::base::SocketReadAwaiter::read_available() {
    int sizeToRead = std::min(size, buffer.get_total_remaining());
    buffer.read(dest + offset, sizeToRead);
    offset += sizeToRead;
    size -= sizeToRead;
}

bool coros::base::SocketReadAwaiter::await_ready() noexcept {
    read_available();
    return size == 0;
}

void coros::base::SocketReadAwaiter::await_suspend(std::coroutine_handle<> handle) {
    read(handle);
}

void coros::base::SocketReadAwaiter::await_resume() {
    if (size > 0) {
        throw error;
    }
}

void coros::base::SocketReadByteAwaiter::read(std::coroutine_handle<> handle) {
    try {
        int status = stream.recv_from_socket(buffer);
        if (status == SOCKET_OP_BLOCK && buffer.get_total_remaining() == 0) {
            return event_manager.set_read_handler([&, handle]() {
                read(handle);
            });
        }
        if (status == SOCKET_OP_CLOSE && buffer.get_total_remaining() == 0) {
            throw std::runtime_error("Socket has been closed");
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

bool coros::base::SocketReadByteAwaiter::await_ready() noexcept {
    return buffer.get_total_remaining() > 0;
}

void coros::base::SocketReadByteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    read(handle);
}

std::byte coros::base::SocketReadByteAwaiter::await_resume() {
    if (buffer.get_total_remaining() == 0) {
        throw error;
    }
    return buffer.read_b();
}
