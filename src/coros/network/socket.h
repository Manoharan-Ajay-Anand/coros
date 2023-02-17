#ifndef COROS_SOCKET_H
#define COROS_SOCKET_H

#include "coros/async/future.h"
#include "coros/memory/buffer.h"
#include "stream.h"
#include "socket_op.h"

#include <atomic>
#include <cstddef>
#include <sys/socket.h>

namespace coros::base {
    class IoEventMonitor;

    class IoEventListener;

    struct SocketDetails {
        int socket_fd;
        sockaddr_storage client_addr;
        socklen_t addr_size;
    };

    class Socket {
        private:
            SocketDetails details;
            IoEventMonitor& io_monitor;
            IoEventListener* io_listener;
            SocketStream stream;
            ByteBuffer input_buffer;
            ByteBuffer output_buffer;
            std::atomic_bool is_closed;
            void read_available(std::byte*& dest, long long& size, long long& total_read);
            void skip_available(long long& size, long long& total_skipped);
            void write_available(std::byte*& src, long long& size);
        public:
            Socket(SocketDetails details, IoEventMonitor& io_monitor);
            AwaitableValue<long long> read(std::byte* dest, long long size, bool read_fully);
            AwaitableValue<long long> skip(long long size, bool skip_fully);
            AwaitableFuture write(std::byte* src, long long size);
            AwaitableFuture flush();
            int get_fd();
            void close_socket();
    };
}

#endif
