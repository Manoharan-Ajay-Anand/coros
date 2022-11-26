#ifndef COROS_SOCKET_H
#define COROS_SOCKET_H

#include "coros/event/monitor.h"
#include "coros/event/manager.h"
#include "coros/async/read_awaiter.h"
#include "coros/async/write_awaiter.h"
#include "coros/memory/buffer.h"
#include "stream.h"

#include <atomic>
#include <cstddef>
#include <sys/socket.h>

namespace coros::async {
    class ThreadPool;
}

namespace coros::network {
    struct SocketDetails {
        int socket_fd;
        sockaddr_storage client_addr;
        socklen_t addr_size;
    };

    class Socket {
        private:
            SocketDetails details;
            event::SocketEventManager event_manager;
            SocketStream stream;
            memory::ByteBuffer input_buffer;
            memory::ByteBuffer output_buffer;
        public:
            Socket(SocketDetails details, event::SocketEventMonitor& event_monitor, 
                   async::ThreadPool& thread_pool);
            async::SocketReadAwaiter read(std::byte* dest, int size);
            async::SocketReadByteAwaiter read_b();
            async::SocketWriteAwaiter write(std::byte* src, int size);
            async::SocketWriteByteAwaiter write_b(const std::byte b);
            async::SocketFlushAwaiter flush();
            int get_fd();
            void close_socket();
    };
} 

#endif
