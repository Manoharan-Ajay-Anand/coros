#ifndef COROS_ASYNC_READ_AWAITER_H
#define COROS_ASYNC_READ_AWAITER_H

#include <cstdint>
#include <coroutine>
#include <stdexcept>
#include <mutex>

namespace coros {
    namespace memory {
        class SocketReadBuffer;
    }

    class Socket;

    namespace async {
        struct SocketReadAwaiter {
            Socket& socket;
            memory::SocketReadBuffer& buffer;
            uint8_t* dest;
            int offset;
            int size;
            std::runtime_error error;
            void read(std::coroutine_handle<> handle);
            void read_available();
            bool await_ready() noexcept;
            void await_suspend(std::coroutine_handle<> handle);
            void await_resume();
        };

        struct SocketReadByteAwaiter {
            Socket& socket;
            memory::SocketReadBuffer& buffer;
            std::runtime_error error;
            void read(std::coroutine_handle<> handle);
            bool await_ready() noexcept;
            void await_suspend(std::coroutine_handle<> handle);
            uint8_t await_resume();
        };
    }
}

#endif
