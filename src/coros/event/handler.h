#ifndef COROS_EVENT_HANDLER_H
#define COROS_EVENT_HANDLER_H

namespace coros::base {
    class SocketEventHandler {
        public:
            virtual void on_socket_event(bool can_read, bool can_write) = 0;
    };
}

#endif
