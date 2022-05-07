#ifndef MEMORY_READ_SOCKET_BUFFER_H
#define MEMORY_READ_SOCKET_BUFFER_H

#include "socket_buffer.h"

#include <cstdint>

namespace server {
    namespace memory {
        class SocketReadBuffer : public SocketBuffer {
            public:
                SocketReadBuffer(int socket_fd);
                void read(uint8_t* dest, int size);
                int recv_socket();
        };
    }
}

#endif
