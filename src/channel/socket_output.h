#ifndef CHANNEL_SOCKET_OUTPUT_H
#define CHANNEL_SOCKET_OUTPUT_H

#include "memory/socket_buffer.h"
#include "concurrent/coroutine.h"

#include <cstdint>
#include <coroutine>

namespace server {
    class Socket;
}

namespace channel {
    class SocketOutputChannel {
        private:
            server::Socket& socket;
            memory::SocketBuffer buffer;
            void write_buffer_from_mem(std::coroutine_handle<> handle, uint8_t* src, 
                int offset, int size);
            void flush_buffer(std::coroutine_handle<> handle);
        public:
            SocketOutputChannel(server::Socket& socket);
            concurrent::Awaiter write_from_mem(uint8_t* src, int size);
            concurrent::Awaiter flush();
    };
}

#endif
