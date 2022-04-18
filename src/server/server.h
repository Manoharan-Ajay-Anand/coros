#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include "socket.h"
#include "event/event.h"
#include "concurrent/thread_pool.h"

#include <memory>
#include <unordered_map>
#include <string>

struct addrinfo;

namespace server {
    class Server : public event::SocketEventHandler {
        private:
            event::SocketEventMonitor event_monitor;
            concurrent::ThreadPool thread_pool;
            std::unordered_map<int, std::unique_ptr<Socket>> socket_map;
            std::string service;
            int server_socketfd;
            addrinfo get_local_addr_info();
            void set_non_blocking(int socket_fd);
        public:
            Server(short port);
            ~Server();
            void on_socket_event(bool can_read, bool can_write);
            void bootstrap();
    };
}

#endif
