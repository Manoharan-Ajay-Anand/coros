#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "event/event.h"
#include "channel/socket_input.h"
#include "channel/socket_output.h"
#include "concurrent/coroutine.h"

#include <functional>
#include <sys/socket.h>

namespace concurrent {
    class ThreadPool;
}

namespace server {
    class Server;
    class Socket : public event::SocketEventHandler {
        private:
            int socket_fd;
            sockaddr_storage client_addr;
            socklen_t addr_size;
            Server& server;
            event::SocketEventMonitor& event_monitor;
            concurrent::ThreadPool& thread_pool;
            channel::SocketInputChannel input_channel;
            channel::SocketOutputChannel output_channel;
            std::function<void()> read_handler;
            bool read_handler_set;
            std::function<void()> write_handler;
            bool write_handler_set;
        public:
            Socket(int socket_fd, sockaddr_storage client_addr, socklen_t addr_size, 
                Server& server, event::SocketEventMonitor& event_monitor, 
                concurrent::ThreadPool& thread_pool);
            ~Socket();
            int get_fd();
            void on_socket_event(bool can_read, bool can_write);
            void set_read_handler(std::function<void()> read_handler);
            void set_write_handler(std::function<void()> write_handler);
            concurrent::Future handle_request();
    };
} 

#endif
