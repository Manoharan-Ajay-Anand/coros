#ifndef SOCKET_AWAITER_H
#define SOCKET_AWAITER_H

#include <cstdint>
#include <coroutine>

namespace server {
    namespace memory {
        class SocketReadBuffer;

        class SocketWriteBuffer;
    }

    class Socket;

    struct SocketReadAwaiter {
        Socket& socket;
        memory::SocketReadBuffer& buffer;
        uint8_t* dest;
        int offset;
        int size;
        void read(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        constexpr void await_resume() const noexcept {}
    };

    struct SocketWriteAwaiter {
        Socket& socket;
        memory::SocketWriteBuffer& buffer;
        uint8_t* src;
        int offset;
        int size;
        void write(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        constexpr void await_resume() const noexcept {}
    };

    struct SocketFlushAwaiter {
        Socket& socket;
        memory::SocketWriteBuffer& buffer;
        void flush(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        constexpr void await_resume() const noexcept {}
    };
}

#endif
