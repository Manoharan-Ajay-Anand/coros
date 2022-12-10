#ifndef COROS_SERVER_H
#define COROS_SERVER_H

#include "coros/event/handler.h"
#include "coros/async/future.h"

#include <string>
#include <netdb.h>

namespace coros::base {
    class ServerApplication;

    class ThreadPool;

    class SocketEventMonitor;

    class Server : public SocketEventHandler {
        private:
            ServerApplication& server_app;
            SocketEventMonitor& event_monitor;
            ThreadPool& thread_pool;
            std::string service;
            int server_socketfd;
            addrinfo* get_local_addr_info();
            void set_non_blocking(int socket_fd);
        public:
            Server(short port, ServerApplication& server_app, SocketEventMonitor& event_monitor, 
                   ThreadPool& thread_pool);
            void on_socket_event(bool can_read, bool can_write);
            void setup();
            void start(bool start_async);
            void shutdown();
    };
}

#endif
