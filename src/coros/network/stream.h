#ifndef COROS_NETWORK_STREAM_H
#define COROS_NETWORK_STREAM_H

#include "socket_op.h"

#include <atomic>

namespace coros::base {
    class ByteBuffer;

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
