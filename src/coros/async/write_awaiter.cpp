#include "write_awaiter.h"
#include "coros/event/manager.h"
#include "coros/memory/buffer.h"
#include "coros/network/stream.h"

#include <algorithm>
#include <coroutine>
#include <iostream>
#include <stdexcept>

void coros::async::SocketWriteAwaiter::write(std::coroutine_handle<> handle) {
    try {
        while (size > 0) {
            int status = stream.send_to_socket(buffer);
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

void coros::async::SocketWriteAwaiter::write_available() {
    int size_to_write = std::min(buffer.get_total_capacity(), size);
    buffer.write(src + offset, size_to_write);
    offset += size_to_write;
    size -= size_to_write;
}

bool coros::async::SocketWriteAwaiter::await_ready() noexcept {
    write_available();
    return size == 0;
}

void coros::async::SocketWriteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    write(handle);
}

void coros::async::SocketWriteAwaiter::await_resume() {
    if (size > 0) {
        throw error;
    }
}

void coros::async::SocketWriteByteAwaiter::write(std::coroutine_handle<> handle) {
    try {
        int status = stream.send_to_socket(buffer);;
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

bool coros::async::SocketWriteByteAwaiter::await_ready() noexcept {
    return buffer.get_total_capacity() > 0;
}

void coros::async::SocketWriteByteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    write(handle);
}

void coros::async::SocketWriteByteAwaiter::await_resume() {
    if (buffer.get_total_capacity() == 0) {
        throw error;
    }
    buffer.write_b(b);
}

void coros::async::SocketFlushAwaiter::flush(std::coroutine_handle<> handle) {
    try {
        int status = stream.send_to_socket(buffer);
        if (status == SOCKET_OP_BLOCK) {
            return event_manager.set_write_handler([&, handle]() {
                flush(handle);
            });
        }
        if (status == SOCKET_OP_CLOSE && buffer.get_total_remaining() > 0) {
            throw std::runtime_error("Socket has been closed");
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

bool coros::async::SocketFlushAwaiter::await_ready() noexcept {
    return buffer.get_total_remaining() == 0;
}

void coros::async::SocketFlushAwaiter::await_suspend(std::coroutine_handle<> handle) {
    flush(handle);
}

void coros::async::SocketFlushAwaiter::await_resume() {
    if (buffer.get_total_remaining() > 0) {
        throw error;
    }
}
