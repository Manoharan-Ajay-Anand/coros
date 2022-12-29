#ifndef COROS_ASYNC_WRITE_AWAITER_H
#define COROS_ASYNC_WRITE_AWAITER_H

#include <cstddef>
#include <coroutine>
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
        std::runtime_error error;
        void write(std::coroutine_handle<> handle);
        void write_available();
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };

    struct SocketWriteByteAwaiter {
        SocketStream& stream;
        SocketEventManager& event_manager;
        ByteBuffer& buffer;
        const std::byte b;
        std::runtime_error error;
        void write(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };

    struct SocketFlushAwaiter {
        SocketStream& stream;
        SocketEventManager& event_manager;
        ByteBuffer& buffer;
        std::runtime_error error;
        void flush(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };
}

#endif
