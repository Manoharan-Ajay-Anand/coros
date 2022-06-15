#ifndef COROS_SERVER_H
#define COROS_SERVER_H

#include "event/event.h"
#include "async/future.h"

#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include <netdb.h>
#include <coroutine>

namespace coros {
    namespace async {
        class ThreadPool;
    }

    class Socket;

    class ServerApplication {
        public:
            virtual async::Future handle_socket(Socket* socket) = 0;
    };
    
    class Server : public event::SocketHandler {
        private:
            ServerApplication& server_app;
            event::SocketEventMonitor& event_monitor;
            async::ThreadPool& thread_pool;
            std::mutex socket_mutex;
            std::unordered_map<int, std::unique_ptr<Socket>> socket_map;
            std::string service;
            int server_socketfd;
            addrinfo* get_local_addr_info();
            void set_non_blocking(int socket_fd);
        public:
            Server(short port, ServerApplication& server_app, 
                   event::SocketEventMonitor& event_monitor, async::ThreadPool& thread_pool);
            void on_socket_event(bool can_read, bool can_write);
            void destroy_socket(int socket_fd);
            void bootstrap();
            void shutdown();
    };
}

#endif
