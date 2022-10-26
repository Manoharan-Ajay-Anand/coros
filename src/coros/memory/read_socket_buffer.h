#ifndef COROS_MEMORY_READ_SOCKET_BUFFER_H
#define COROS_MEMORY_READ_SOCKET_BUFFER_H

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
        class SocketReadBuffer : public SocketBuffer {
            private:
                int recv_size(int size);
            public:
                SocketReadBuffer(int socket_fd);
                uint8_t read_b();
                void read(uint8_t* dest, int size);
                int recv_socket();
        };
    }
}

#endif
