#include "skip_awaiter.h"
#include "coros/event/manager.h"
#include "coros/memory/buffer.h"
#include "coros/network/stream.h"

#include <algorithm>
#include <cstddef>
#include <coroutine>
#include <optional>
#include <stdexcept>

coros::base::SocketSkipAwaiter::SocketSkipAwaiter(SocketStream& stream, 
                                                  SocketEventManager& event_manager,
                                                  ByteBuffer& buffer, long long size)
        : stream(stream), event_manager(event_manager), buffer(buffer), size(size) {
}

void coros::base::SocketSkipAwaiter::skip(std::coroutine_handle<> handle) {
    try {
        while (size > 0) {
            SocketOperation status = stream.recv_from_socket(buffer);
            skip_available();
            if (status == SOCKET_OP_BLOCK && size > 0) {
                return event_manager.set_read_handler([&, handle]() {
                    skip(handle);
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

void coros::base::SocketSkipAwaiter::skip_available() {
    long long size_to_skip = std::min(size, buffer.get_total_remaining());
    buffer.increment_read_pointer(size_to_skip);
    size -= size_to_skip;
}

bool coros::base::SocketSkipAwaiter::await_ready() noexcept {
    skip_available();
    return size == 0;
}

void coros::base::SocketSkipAwaiter::await_suspend(std::coroutine_handle<> handle) {
    skip(handle);
}

void coros::base::SocketSkipAwaiter::await_resume() {
    if (error_optional) {
        throw error_optional.value();
    }
}
