#ifndef COROS_SOCKET_H
#define COROS_SOCKET_H

#include "event/monitor.h"
#include "event/manager.h"
#include "async/read_awaiter.h"
#include "async/write_awaiter.h"
#include "memory/read_socket_buffer.h"
#include "memory/write_socket_buffer.h"


#include <atomic>
#include <cstdint>
#include <sys/socket.h>

namespace coros {
    namespace async {
        class ThreadPool;
    }
    
    struct SocketDetails {
        int socket_fd;
        sockaddr_storage client_addr;
        socklen_t addr_size;
    };

    class Socket {
        private:
            SocketDetails details;
            event::SocketEventManager event_manager;
            memory::SocketReadBuffer input_buffer;
            memory::SocketWriteBuffer output_buffer;
        public:
            Socket(SocketDetails details, event::SocketEventMonitor& event_monitor, 
                   async::ThreadPool& thread_pool);
            async::SocketReadAwaiter read(uint8_t* dest, int size);
            async::SocketReadByteAwaiter read_b();
            async::SocketWriteAwaiter write(const uint8_t* src, int size);
            async::SocketWriteByteAwaiter write_b(const uint8_t b);
            async::SocketFlushAwaiter flush();
            int get_fd();
            void close_socket();
    };
} 

#endif
