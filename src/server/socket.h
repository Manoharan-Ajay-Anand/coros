#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "event/event.h"
#include "concurrent/awaiter.h"
#include "memory/read_socket_buffer.h"
#include "memory/write_socket_buffer.h"

#include <cstdint>
#include <functional>
#include <sys/socket.h>
#include <mutex>
#include <atomic>

namespace server {
    namespace concurrent {
        class ThreadPool;
    }
    
    class Server;

    class Socket : public event::SocketHandler {
        private:
            int socket_fd;
            sockaddr_storage client_addr;
            socklen_t addr_size;
            Server& server;
            event::SocketEventMonitor& event_monitor;
            concurrent::ThreadPool& thread_pool;
            memory::SocketReadBuffer input_buffer;
            memory::SocketWriteBuffer output_buffer;
            std::atomic_bool marked_for_close;
            std::mutex read_handler_mutex;
            bool read_handler_set;
            std::function<void()> read_handler;
            std::mutex write_handler_mutex;
            bool write_handler_set;
            std::function<void()> write_handler;
            void on_socket_read(bool can_read);
            void on_socket_write(bool can_write);
        public:
            Socket(int socket_fd, sockaddr_storage client_addr, socklen_t addr_size, 
                Server& server, event::SocketEventMonitor& event_monitor, 
                concurrent::ThreadPool& thread_pool);
            ~Socket();
            void listen_for_read(std::function<void()> handler);
            void listen_for_write(std::function<void()> handler);
            void on_socket_event(bool can_read, bool can_write);
            SocketReadAwaiter read(uint8_t* dest, int size);
            SocketWriteAwaiter write(uint8_t* src, int size);
            SocketFlushAwaiter flush();
            void close_socket();
    };
} 

#endif
