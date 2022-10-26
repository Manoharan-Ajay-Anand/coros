#include "write_awaiter.h"
#include "coros/socket.h"
#include "coros/memory/write_socket_buffer.h"

#include <algorithm>
#include <coroutine>
#include <iostream>
#include <stdexcept>

void coros::async::SocketWriteAwaiter::write(std::coroutine_handle<> handle) {
    try {
        while (size > 0) {
            int status = buffer.send_socket();
            write_available();
            if (status == SOCKET_OP_WOULD_BLOCK && size > 0) {
                return event_manager.set_write_handler([&, handle]() {
                    write(handle);
                });
            }
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

void coros::async::SocketWriteAwaiter::write_available() {
    int size_to_write = std::min(buffer.capacity(), size);
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
        int status = buffer.send_socket();
        if (status == SOCKET_OP_WOULD_BLOCK && buffer.capacity() == 0) {
            return event_manager.set_write_handler([&, handle]() {
                write(handle);
            });
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

bool coros::async::SocketWriteByteAwaiter::await_ready() noexcept {
    return buffer.capacity() > 0;
}

void coros::async::SocketWriteByteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    write(handle);
}

void coros::async::SocketWriteByteAwaiter::await_resume() {
    if (buffer.capacity() == 0) {
        throw error;
    }
    buffer.write_b(b);
}

void coros::async::SocketFlushAwaiter::flush(std::coroutine_handle<> handle) {
    try {
        int status = buffer.send_socket();
        if (status == SOCKET_OP_WOULD_BLOCK) {
            return event_manager.set_write_handler([&, handle]() {
                flush(handle);
            });
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

bool coros::async::SocketFlushAwaiter::await_ready() noexcept {
    return buffer.remaining() == 0;
}

void coros::async::SocketFlushAwaiter::await_suspend(std::coroutine_handle<> handle) {
    flush(handle);
}

void coros::async::SocketFlushAwaiter::await_resume() {
    if (buffer.remaining() > 0) {
        throw error;
    }
}
