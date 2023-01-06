#ifndef COROS_AWAITER_SKIP_AWAITER_H
#define COROS_AWAITER_SKIP_AWAITER_H

#include <cstddef>
#include <coroutine>
#include <optional>
#include <stdexcept>

namespace coros::base {
    class SocketEventManager;

    class ByteBuffer;

    class SocketStream;

    struct SocketSkipAwaiter {
        SocketStream& stream;
        SocketEventManager& event_manager;
        ByteBuffer& buffer;
        const long long size;
        long long total_size_skipped;
        const bool skip_fully;
        std::optional<std::runtime_error> error_optional;
        SocketSkipAwaiter(SocketStream& stream, SocketEventManager& event_manager,
                          ByteBuffer& buffer, long long size, bool skip_fully);
        void skip(std::coroutine_handle<> handle);
        void skip_available();
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        long long await_resume();
    };
}
#endif
