#ifndef COROS_ASYNC_READ_AWAITER_H
#define COROS_ASYNC_READ_AWAITER_H

#include <cstddef>
#include <coroutine>
#include <stdexcept>

namespace coros::event {
    class SocketEventManager;
}

namespace coros::memory {
    class ByteBuffer;
}

namespace coros::network {
    class SocketStream;
}

namespace coros::async {
    struct SocketReadAwaiter {
        network::SocketStream& stream;
        event::SocketEventManager& event_manager;
        memory::ByteBuffer& buffer;
        std::byte* dest;
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
        network::SocketStream& stream;
        event::SocketEventManager& event_manager;
        memory::ByteBuffer& buffer;
        std::runtime_error error;
        void read(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        std::byte await_resume();
    };
}

#endif
