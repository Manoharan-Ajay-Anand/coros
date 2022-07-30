#ifndef COROS_MEMORY_WRITE_SOCKET_BUFFER_H
#define COROS_MEMORY_WRITE_SOCKET_BUFFER_H

#include "socket_buffer.h"

#include <cstdint>

namespace coros {
    namespace memory {
        class SocketWriteBuffer : public SocketBuffer {
            public:
                SocketWriteBuffer(int socket_fd);
                void write(const uint8_t* src, int size);
                void write_b(const uint8_t b);
                int send_socket();
        };
    }
}

#endif
