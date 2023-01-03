#ifndef COROS_AWAITER_READ_AWAITER_H
#define COROS_AWAITER_READ_AWAITER_H

#include <cstddef>
#include <coroutine>
#include <optional>
#include <stdexcept>

namespace coros::base {
    class SocketEventManager;

    class ByteBuffer;

    class SocketStream;

    struct SocketReadAwaiter {
        SocketStream& stream;
        SocketEventManager& event_manager;
        ByteBuffer& buffer;
        std::byte* dest;
        long long size;
        std::optional<std::runtime_error> error_optional;
        SocketReadAwaiter(SocketStream& stream, SocketEventManager& event_manager,
                          ByteBuffer& buffer, std::byte* dest, long long size);
        void read(std::coroutine_handle<> handle);
        void read_available();
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };

    struct SocketReadByteAwaiter {
        SocketStream& stream;
        SocketEventManager& event_manager;
        ByteBuffer& buffer;
        std::optional<std::runtime_error> error_optional;
        SocketReadByteAwaiter(SocketStream& stream, SocketEventManager& event_manager,
                              ByteBuffer& buffer);
        void read(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        std::byte await_resume();
    };
}

#endif
