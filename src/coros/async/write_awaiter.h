#ifndef COROS_ASYNC_WRITE_AWAITER_H
#define COROS_ASYNC_WRITE_AWAITER_H

#include <cstdint>
#include <coroutine>
#include <stdexcept>

namespace coros {
    namespace memory {
        class SocketWriteBuffer;
    }

    class Socket;

    namespace async {
        struct SocketWriteAwaiter {
            Socket& socket;
            memory::SocketWriteBuffer& buffer;
            uint8_t* src;
            int offset;
            int size;
            std::runtime_error error;
            void write(std::coroutine_handle<> handle);
            void write_available();
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
}

#endif
