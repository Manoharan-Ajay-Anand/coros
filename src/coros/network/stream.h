#ifndef COROS_NETWORK_STREAM_H
#define COROS_NETWORK_STREAM_H

#include <atomic>

namespace coros::base {
    class ByteBuffer;

    enum SocketOperation {
        SOCKET_OP_BLOCK,
        SOCKET_OP_CLOSE,
        SOCKET_OP_SUCCESS
    };

    class SocketStream {
        private:
            int socket_fd;
            std::atomic_bool is_closed;
        public:
            SocketStream(int socket_fd);
            SocketOperation recv_from_socket(ByteBuffer& buffer);
            SocketOperation send_to_socket(ByteBuffer& buffer);
            void close();
    }; 
}


#endif
