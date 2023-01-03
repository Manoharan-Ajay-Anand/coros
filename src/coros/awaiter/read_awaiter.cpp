#include "read_awaiter.h"
#include "coros/event/manager.h"
#include "coros/memory/buffer.h"
#include "coros/network/stream.h"

#include <algorithm>
#include <coroutine>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <cstddef>

coros::base::SocketReadAwaiter::SocketReadAwaiter(SocketStream& stream, 
                                                  SocketEventManager& event_manager,
                                                  ByteBuffer& buffer, 
                                                  std::byte* dest, long long size) 
        : stream(stream), event_manager(event_manager), buffer(buffer), dest(dest), size(size) {
}

void coros::base::SocketReadAwaiter::read(std::coroutine_handle<> handle) {
    try {
        while (size > 0) {
            SocketOperation status = stream.recv_from_socket(buffer);
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
        error_optional = error;
    }
    handle.resume();
}

void coros::base::SocketReadAwaiter::read_available() {
    long long size_to_read = std::min(size, buffer.get_total_remaining());
    buffer.read(dest, size_to_read);
    dest += size_to_read;
    size -= size_to_read;
}

bool coros::base::SocketReadAwaiter::await_ready() noexcept {
    read_available();
    return size == 0;
}

void coros::base::SocketReadAwaiter::await_suspend(std::coroutine_handle<> handle) {
    read(handle);
}

void coros::base::SocketReadAwaiter::await_resume() {
    if (error_optional) {
        throw error_optional.value();
    }
}

coros::base::SocketReadByteAwaiter::SocketReadByteAwaiter(SocketStream& stream, 
                                                          SocketEventManager& event_manager,
                                                          ByteBuffer& buffer) 
        : stream(stream), event_manager(event_manager), buffer(buffer) {
}

void coros::base::SocketReadByteAwaiter::read(std::coroutine_handle<> handle) {
    try {
        SocketOperation status = stream.recv_from_socket(buffer);
        if (status == SOCKET_OP_BLOCK && buffer.get_total_remaining() == 0) {
            return event_manager.set_read_handler([&, handle]() {
                read(handle);
            });
        }
        if (status == SOCKET_OP_CLOSE && buffer.get_total_remaining() == 0) {
            throw std::runtime_error("Socket has been closed");
        }
    } catch (std::runtime_error error) {
        error_optional = error;
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
    if (error_optional) {
        throw error_optional.value();
    }
    return buffer.read_b();
}
