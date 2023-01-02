#include "write_awaiter.h"
#include "coros/event/manager.h"
#include "coros/memory/buffer.h"
#include "coros/network/stream.h"

#include <algorithm>
#include <coroutine>
#include <iostream>
#include <stdexcept>

void coros::base::SocketWriteAwaiter::write(std::coroutine_handle<> handle) {
    try {
        while (size > 0) {
            SocketOperation status = stream.send_to_socket(buffer);
            write_available();
            if (status == SOCKET_OP_BLOCK && size > 0) {
                return event_manager.set_write_handler([&, handle]() {
                    write(handle);
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

void coros::base::SocketWriteAwaiter::write_available() {
    long long size_to_write = std::min(buffer.get_total_capacity(), size);
    buffer.write(src, size_to_write);
    src += size_to_write;
    size -= size_to_write;
}

bool coros::base::SocketWriteAwaiter::await_ready() noexcept {
    write_available();
    return size == 0;
}

void coros::base::SocketWriteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    write(handle);
}

void coros::base::SocketWriteAwaiter::await_resume() {
    if (size > 0) {
        throw error;
    }
}

void coros::base::SocketWriteByteAwaiter::write(std::coroutine_handle<> handle) {
    try {
        SocketOperation status = stream.send_to_socket(buffer);
        if (status == SOCKET_OP_BLOCK && buffer.get_total_capacity() == 0) {
            return event_manager.set_write_handler([&, handle]() {
                write(handle);
            });
        }
        if (status == SOCKET_OP_CLOSE && buffer.get_total_capacity() == 0) {
            throw std::runtime_error("Socket has been closed");
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

bool coros::base::SocketWriteByteAwaiter::await_ready() noexcept {
    return buffer.get_total_capacity() > 0;
}

void coros::base::SocketWriteByteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    write(handle);
}

void coros::base::SocketWriteByteAwaiter::await_resume() {
    if (buffer.get_total_capacity() == 0) {
        throw error;
    }
    buffer.write_b(b);
}
