#ifndef COROS_SERVER_H
#define COROS_SERVER_H

#include "event/event.h"
#include "async/future.h"

#include <string>
#include <netdb.h>

namespace coros {
    class ServerApplication;

    namespace async {
        class ThreadPool;
    }
    
    class Server : public event::SocketHandler {
        private:
            ServerApplication& server_app;
            event::SocketEventMonitor& event_monitor;
            async::ThreadPool& thread_pool;
            std::string service;
            int server_socketfd;
            addrinfo* get_local_addr_info();
            void set_non_blocking(int socket_fd);
        public:
            Server(short port, ServerApplication& server_app,
                   event::SocketEventMonitor& event_monitor, async::ThreadPool& thread_pool);
            void on_socket_event(bool can_read, bool can_write);
            void setup();
            void start(bool start_async);
            void shutdown();
    };
}

#endif
