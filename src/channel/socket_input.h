#ifndef CHANNEL_SOCKET_INPUT_H
#define CHANNEL_SOCKET_INPUT_H

#include "memory/socket_buffer.h"
#include "concurrent/coroutine.h"

#include <cstdint>
#include <coroutine>

namespace server {
    class Socket;
}

namespace channel {
    class SocketInputChannel {
        private:
            server::Socket& socket;
            memory::SocketBuffer buffer;
            void read_buffer_to_mem(std::coroutine_handle<> handle, uint8_t* dest, 
                int offset, int size);
        public:
            SocketInputChannel();
            SocketInputChannel(server::Socket& socket);
            concurrent::Awaiter read_to_mem(uint8_t* dest, int size);
    };
}

#endif
