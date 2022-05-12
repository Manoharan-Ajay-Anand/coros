#ifndef MEMORY_WRITE_SOCKET_BUFFER_H
#define MEMORY_WRITE_SOCKET_BUFFER_H

#include "socket_buffer.h"

#include <cstdint>

namespace coros {
    namespace memory {
        class SocketWriteBuffer : public SocketBuffer {
            public:
                SocketWriteBuffer(int socket_fd);
                void write(uint8_t* src, int size);
                int send_socket();
        };
    }
}

#endif
