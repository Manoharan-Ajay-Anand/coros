#ifndef COROS_MEMORY_WRITE_SOCKET_BUFFER_H
#define COROS_MEMORY_WRITE_SOCKET_BUFFER_H

#include "socket_buffer.h"

#include <cstdint>
#include <optional>
#include <mutex>
#include <functional>

namespace coros {
    class Socket;

    namespace async {
        class ThreadPool;
    }
    
    namespace memory {
        class SocketWriteBuffer : public SocketBuffer {
            private:
                std::mutex write_mutex;
                std::optional<std::function<void()>> write_handler;
                int send_size(int size);
            public:
                SocketWriteBuffer(int socket_fd, async::ThreadPool& thread_pool, Socket& socket);
                void write_b(const uint8_t b);
                void write(const uint8_t* src, int size);
                int send_socket();
                void set_write_handler(std::function<void()> handler);
                void on_write(bool can_write);
                void close();
        };
    }
}

#endif
