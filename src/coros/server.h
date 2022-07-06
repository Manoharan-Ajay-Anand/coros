#ifndef COROS_SERVER_H
#define COROS_SERVER_H

#include "event/event.h"
#include "async/future.h"
#include "async/thread_pool.h"

#include <memory>
#include <string>
#include <netdb.h>
#include <coroutine>

namespace coros {
    class Socket;

    class ServerApplication;
    
    class Server : public event::SocketHandler {
        private:
            ServerApplication& server_app;
            event::SocketEventMonitor event_monitor;
            async::ThreadPool thread_pool;
            std::string service;
            int server_socketfd;
            addrinfo* get_local_addr_info();
            void set_non_blocking(int socket_fd);
        public:
            Server(short port, ServerApplication& server_app);
            void on_socket_event(bool can_read, bool can_write);
            void setup();
            void start(bool start_async);
            void shutdown();
            void run_async(std::function<void()> job);
            void register_socket_event(int socket_fd, SocketHandler& handler);
            void deregister_socket_event(int socket_fd);
    };
}

#endif
