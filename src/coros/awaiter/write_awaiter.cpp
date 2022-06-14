#include "write_awaiter.h"
#include "coros/socket.h"
#include "coros/memory/write_socket_buffer.h"

#include <algorithm>
#include <coroutine>
#include <iostream>
#include <stdexcept>

void coros::awaiter::SocketWriteAwaiter::write(std::coroutine_handle<> handle) {
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
                }, [handle]() { handle.destroy(); });
            }
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

bool coros::awaiter::SocketWriteAwaiter::await_ready() noexcept {
    int size_to_write = std::min(buffer.capacity(), size);
    buffer.write(src + offset, size_to_write);
    offset += size_to_write;
    size -= size_to_write;
    return size == 0;
}

void coros::awaiter::SocketWriteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    write(handle);
}

void coros::awaiter::SocketWriteAwaiter::await_resume() {
    if (size > 0) {
        throw error;
    }
}

void coros::awaiter::SocketFlushAwaiter::flush(std::coroutine_handle<> handle) {
    try {
        while (buffer.remaining() > 0) {
            int status = buffer.send_socket();
            if (status == SOCKET_OP_WOULD_BLOCK) {
                return socket.listen_for_write([&, handle]() {
                    flush(handle);
                }, [handle]() { handle.destroy(); });
            }
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

bool coros::awaiter::SocketFlushAwaiter::await_ready() noexcept {
    return buffer.remaining() == 0;
}

void coros::awaiter::SocketFlushAwaiter::await_suspend(std::coroutine_handle<> handle) {
    flush(handle);
}

void coros::awaiter::SocketFlushAwaiter::await_resume() {
    if (buffer.remaining() > 0) {
        throw error;
    }
}
