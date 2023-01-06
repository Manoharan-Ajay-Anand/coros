#ifndef COROS_AWAITER_WRITE_AWAITER_H
#define COROS_AWAITER_WRITE_AWAITER_H

#include <cstddef>
#include <coroutine>
#include <optional>
#include <stdexcept>

namespace coros::base {
    class SocketEventManager;

    class ByteBuffer;

    class SocketStream;

    struct SocketWriteAwaiter {
        SocketStream& stream;
        SocketEventManager& event_manager;
        ByteBuffer& buffer;
        std::byte* src;
        long long size;
        std::optional<std::runtime_error> error_optional;
        SocketWriteAwaiter(SocketStream& stream, SocketEventManager& event_manager,
                           ByteBuffer& buffer, std::byte* src, long long size);
        void write(std::coroutine_handle<> handle);
        void write_available();
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };
}

#endif
