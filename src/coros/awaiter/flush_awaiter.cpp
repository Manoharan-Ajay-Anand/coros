#include "flush_awaiter.h"
#include "coros/event/manager.h"
#include "coros/memory/buffer.h"
#include "coros/network/stream.h"

#include <coroutine>
#include <iostream>
#include <stdexcept>

coros::base::SocketFlushAwaiter::SocketFlushAwaiter(SocketStream& stream, 
                                                    SocketEventManager& event_manager,
                                                    ByteBuffer& buffer)
        : stream(stream), event_manager(event_manager), buffer(buffer) {
}

void coros::base::SocketFlushAwaiter::flush(std::coroutine_handle<> handle) {
    try {
        SocketOperation status = stream.send_to_socket(buffer);
        if (status == SOCKET_OP_BLOCK) {
            return event_manager.set_write_handler([&, handle]() {
                flush(handle);
            });
        }
        if (status == SOCKET_OP_CLOSE && buffer.get_total_remaining() > 0) {
            throw std::runtime_error("Socket has been closed");
        }
    } catch (std::runtime_error error) {
        error_optional = error;
    }
    handle.resume();
}

bool coros::base::SocketFlushAwaiter::await_ready() noexcept {
    return buffer.get_total_remaining() == 0;
}

void coros::base::SocketFlushAwaiter::await_suspend(std::coroutine_handle<> handle) {
    flush(handle);
}

void coros::base::SocketFlushAwaiter::await_resume() {
    if (error_optional) {
        throw error_optional.value();
    }
}
