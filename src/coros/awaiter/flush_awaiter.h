#ifndef COROS_AWAITER_FLUSH_AWAITER_H
#define COROS_AWAITER_FLUSH_AWAITER_H

#include <cstddef>
#include <coroutine>
#include <optional>
#include <stdexcept>

namespace coros::base {
    class SocketEventManager;

    class ByteBuffer;

    class SocketStream;

    struct SocketFlushAwaiter {
        SocketStream& stream;
        SocketEventManager& event_manager;
        ByteBuffer& buffer;
        std::optional<std::runtime_error> error_optional;
        SocketFlushAwaiter(SocketStream& stream, SocketEventManager& event_manager,
                           ByteBuffer& buffer);
        void flush(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };
}
#endif
