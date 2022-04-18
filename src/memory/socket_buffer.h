#ifndef MEMORY_SOCKET_BUFFER_H
#define MEMORY_SOCKET_BUFFER_H

#include <cstdint>

#define SOCKET_OP_CONTINUE 1
#define SOCKET_OP_WOULD_BLOCK 0

#define BUFFER_LIMIT 8192

namespace memory {
    class SocketBuffer {
        private:
            uint8_t* data;
            int start;
            int end;
            int limit;
            void compact();
        public:
            SocketBuffer();
            ~SocketBuffer();
            int remaining();
            void read(uint8_t* dest, int size);
            int recv_socket(int socket_fd);
            int capacity();
            void write(uint8_t* src, int size);
            int send_socket(int socket_fd);
    };
}

#endif
