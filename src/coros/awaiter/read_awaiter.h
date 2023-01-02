#ifndef COROS_AWAITER_READ_AWAITER_H
#define COROS_AWAITER_READ_AWAITER_H

#include <cstddef>
#include <coroutine>
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
        std::runtime_error error;
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
        std::runtime_error error;
        void read(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        std::byte await_resume();
    };
}

#endif
