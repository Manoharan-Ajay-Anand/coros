#ifndef COROS_NETWORK_STREAM_H
#define COROS_NETWORK_STREAM_H

#define SOCKET_OP_BLOCK 0
#define SOCKET_OP_CLOSE 1
#define SOCKET_OP_SUCCESS 2

#include <atomic>

namespace coros::memory {
    class ByteBuffer;
}

namespace coros::network {
    class SocketStream {
        private:
            int socket_fd;
            std::atomic_bool is_closed;
        public:
            SocketStream(int socket_fd);
            int recv_from_socket(memory::ByteBuffer& buffer);
            int send_to_socket(memory::ByteBuffer& buffer);
            void close();
    }; 
}


#endif
