#ifndef COROS_MEMORY_SOCKET_BUFFER_H
#define COROS_MEMORY_SOCKET_BUFFER_H

#include <cstdint>
#include <array>
#include <atomic>

#define SOCKET_OP_CONTINUE 1
#define SOCKET_OP_WOULD_BLOCK 0

#define BUFFER_LENGTH 8192

namespace coros {
    class Socket;

    namespace async {
        class ThreadPool;
    }
    
    namespace memory {
        class SocketBuffer {
            protected:
                int socket_fd;
                Socket& socket;
                async::ThreadPool& thread_pool;
                std::array<uint8_t, BUFFER_LENGTH> buffer;
                int read_index;
                int write_index;
                std::atomic_bool is_closed;
                SocketBuffer(int socket_fd, async::ThreadPool& thread_pool, Socket& socket);
                int get_position(int index);
                void reset_read_write_indices();
            public:
                int get_fd();
                int remaining();
                int capacity();
                virtual void close() = 0;
        };
    }
}

#endif
