#ifndef COROS_SOCKET_H
#define COROS_SOCKET_H

#include "event/event.h"
#include "async/read_awaiter.h"
#include "async/write_awaiter.h"
#include "memory/read_socket_buffer.h"
#include "memory/write_socket_buffer.h"

#include <cstdint>
#include <functional>
#include <sys/socket.h>
#include <mutex>

namespace coros {
    namespace async {
        class ThreadPool;
    }
    
    struct SocketDetails {
        int socket_fd;
        sockaddr_storage client_addr;
        socklen_t addr_size;
    };

    class Socket : public event::SocketHandler {
        private:
            SocketDetails details;
            event::SocketEventMonitor& event_monitor;
            async::ThreadPool& thread_pool;
            memory::SocketReadBuffer input_buffer;
            memory::SocketWriteBuffer output_buffer;
            std::atomic_bool marked_for_close;
            std::mutex read_mutex;
            bool read_handler_set;
            std::function<void()> read_handler;
            std::mutex write_mutex;
            bool write_handler_set;
            std::function<void()> write_handler;
            void on_socket_read(bool can_read);
            void on_socket_write(bool can_write);
        public:
            Socket(SocketDetails details, event::SocketEventMonitor& event_monitor, 
                   async::ThreadPool& thread_pool);
            void listen_for_read(std::function<void()> handler);
            void listen_for_write(std::function<void()> handler);
            void on_socket_event(bool can_read, bool can_write);
            async::SocketReadAwaiter read(uint8_t* dest, int size);
            async::SocketReadByteAwaiter read_b();
            async::SocketWriteAwaiter write(uint8_t* src, int size);
            async::SocketFlushAwaiter flush();
            int get_fd();
            void close_socket();
    };
} 

#endif
