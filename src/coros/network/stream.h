#ifndef COROS_NETWORK_STREAM_H
#define COROS_NETWORK_STREAM_H

#define SOCKET_OP_BLOCK 0
#define SOCKET_OP_CLOSE 1
#define SOCKET_OP_SUCCESS 2

#include <atomic>

namespace coros::base {
    class ByteBuffer;

    class SocketStream {
        private:
            int socket_fd;
            std::atomic_bool is_closed;
        public:
            SocketStream(int socket_fd);
            int recv_from_socket(ByteBuffer& buffer);
            int send_to_socket(ByteBuffer& buffer);
            void close();
    }; 
}


#endif
