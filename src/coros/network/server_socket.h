#ifndef COROS_NETWORK_SERVER_SOCKET_H
#define COROS_NETWORK_SERVER_SOCKET_H

#include "coros/awaiter/accept_awaiter.h"
#include "coros/event/manager.h"

namespace coros::base {
    class ThreadPool;

    class SocketEventMonitor; 

    class ServerSocket {
        private:
            int socket_fd;
            ThreadPool& thread_pool;
            SocketEventMonitor& event_monitor;
            SocketEventManager event_manager;
        public:
            ServerSocket(short port, ThreadPool& thread_pool, SocketEventMonitor& event_monitor);
            SocketAcceptAwaiter accept();
            void close_socket();
    };
}

#endif
