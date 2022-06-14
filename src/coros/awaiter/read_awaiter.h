#ifndef COROS_AWAITER_READ_AWAITER_H
#define COROS_AWAITER_READ_AWAITER_H

#include <cstdint>
#include <coroutine>
#include <stdexcept>

namespace coros {
    namespace memory {
        class SocketReadBuffer;
    }

    class Socket;

    namespace awaiter {
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
    }
}

#endif
