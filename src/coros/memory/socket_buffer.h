#ifndef MEMORY_SOCKET_BUFFER_H
#define MEMORY_SOCKET_BUFFER_H

#include <cstdint>

#define SOCKET_OP_CONTINUE 1
#define SOCKET_OP_WOULD_BLOCK 0

#define BUFFER_LIMIT 8192

namespace coros {
    namespace memory {
        class SocketBuffer {
            protected:
                int socket_fd;
                uint8_t* data;
                int start;
                int end;
                int limit;
                SocketBuffer(int socket_fd);
                void compact();
            public:
                ~SocketBuffer();
                int get_fd();
                int remaining();
                int capacity();
        };
    }
}

#endif
