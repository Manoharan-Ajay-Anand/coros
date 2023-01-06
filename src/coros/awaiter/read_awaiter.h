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
        const long long size;
        long long total_size_read;
        const bool read_fully;
        std::optional<std::runtime_error> error_optional;
        SocketReadAwaiter(SocketStream& stream, SocketEventManager& event_manager,
                          ByteBuffer& buffer, std::byte* dest, long long size, bool read_fully);
        void read(std::coroutine_handle<> handle);
        void read_available();
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        long long await_resume();
    };
}

#endif
