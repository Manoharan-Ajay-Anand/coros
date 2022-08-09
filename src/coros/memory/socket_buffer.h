#ifndef COROS_MEMORY_SOCKET_BUFFER_H
#define COROS_MEMORY_SOCKET_BUFFER_H

#include <cstdint>
#include <array>
#include <atomic>

#define SOCKET_OP_CONTINUE 1
#define SOCKET_OP_WOULD_BLOCK 0

#define BUFFER_LIMIT 8192

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
                std::array<uint8_t, BUFFER_LIMIT> buffer;
                int start;
                int end;
                std::atomic_bool is_closed;
                SocketBuffer(int socket_fd, async::ThreadPool& thread_pool, Socket& socket);
                void compact();
            public:
                int get_fd();
                int remaining();
                int capacity();
                virtual void close() = 0;
        };
    }
}

#endif
