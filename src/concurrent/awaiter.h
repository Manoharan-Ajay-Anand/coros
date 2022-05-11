#ifndef CONCURRENT_AWAITER_H
#define CONCURRENT_AWAITER_H

#include <cstdint>
#include <coroutine>
#include <stdexcept>

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
        std::runtime_error error;
        void read(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };

    struct SocketWriteAwaiter {
        Socket& socket;
        memory::SocketWriteBuffer& buffer;
        uint8_t* src;
        int offset;
        int size;
        std::runtime_error error;
        void write(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };

    struct SocketFlushAwaiter {
        Socket& socket;
        memory::SocketWriteBuffer& buffer;
        std::runtime_error error;
        void flush(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };
}

#endif
