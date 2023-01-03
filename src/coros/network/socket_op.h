#ifndef COROS_NETWORK_SOCKET_OP_H
#define COROS_NETWORK_SOCKET_OP_H

namespace coros::base {
    enum SocketOperation {
        SOCKET_OP_BLOCK,
        SOCKET_OP_ERROR,
        SOCKET_OP_CLOSE,
        SOCKET_OP_SUCCESS
    };
}

#endif
