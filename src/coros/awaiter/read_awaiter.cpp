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
                                                  std::byte* dest, long long size, 
                                                  bool read_fully) 
        : stream(stream), event_manager(event_manager), buffer(buffer), dest(dest), size(size),
          read_fully(read_fully) {
    total_size_read = 0;
}

void coros::base::SocketReadAwaiter::read(std::coroutine_handle<> handle) {
    try {
        while (total_size_read < size) {
            SocketOperation status = stream.recv_from_socket(buffer);
            read_available();
            if (status == SOCKET_OP_BLOCK && total_size_read < size) {
                return event_manager.set_read_handler([&, handle]() {
                    read(handle);
                });
            }
            if (status == SOCKET_OP_CLOSE && total_size_read < size) {
                if (read_fully) {
                    throw std::runtime_error("Socket read(): Socket has been closed");
                }
                break;
            }
        }
    } catch (std::runtime_error error) {
        error_optional = error;
    }
    handle.resume();
}

void coros::base::SocketReadAwaiter::read_available() {
    long long size_to_read = std::min(size - total_size_read, buffer.get_total_remaining());
    buffer.read(dest, size_to_read);
    dest += size_to_read;
    total_size_read += size_to_read;
}

bool coros::base::SocketReadAwaiter::await_ready() noexcept {
    read_available();
    return total_size_read == size;
}

void coros::base::SocketReadAwaiter::await_suspend(std::coroutine_handle<> handle) {
    read(handle);
}

long long coros::base::SocketReadAwaiter::await_resume() {
    if (error_optional) {
        throw error_optional.value();
    }
    return total_size_read;
}
