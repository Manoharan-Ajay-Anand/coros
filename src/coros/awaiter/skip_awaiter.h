#ifndef COROS_AWAITER_SKIP_AWAITER_H
#define COROS_AWAITER_SKIP_AWAITER_H

#include <cstddef>
#include <coroutine>
#include <stdexcept>

namespace coros::base {
    class SocketEventManager;

    class ByteBuffer;

    class SocketStream;

    struct SocketSkipAwaiter {
        SocketStream& stream;
        SocketEventManager& event_manager;
        ByteBuffer& buffer;
        long long size;
        std::runtime_error error;
        void skip(std::coroutine_handle<> handle);
        void skip_available();
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };
}
#endif
