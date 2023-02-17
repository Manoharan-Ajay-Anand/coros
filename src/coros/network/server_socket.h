#ifndef COROS_NETWORK_SERVER_SOCKET_H
#define COROS_NETWORK_SERVER_SOCKET_H

#include "coros/async/future.h"

#include <memory>

namespace coros::base {
    class IoEventMonitor;

    class IoEventListener; 

    class Socket;

    class ServerSocket {
        private:
            int socket_fd;
            IoEventMonitor& io_monitor;
            IoEventListener* io_listener;
        public:
            ServerSocket(short port, IoEventMonitor& io_monitor);
            AwaitableValue<std::shared_ptr<Socket>> accept_conn();
            void close_socket();
    };
}

#endif
