#ifndef COROS_SERVER_H
#define COROS_SERVER_H

#include "coros/event/handler.h"
#include "coros/async/future.h"
#include "coros/network/server_socket.h"

#include <atomic>
#include <string>

namespace coros::base {
    class ServerApplication;

    class ThreadPool;

    class SocketEventMonitor;

    class Server {
        private:
            ServerApplication& server_app;
            SocketEventMonitor& event_monitor;
            ThreadPool& thread_pool;
            ServerSocket server_socket;
            std::atomic_bool marked_for_close;
            Future run_server_loop();
        public:
            Server(short port, ServerApplication& server_app, SocketEventMonitor& event_monitor, 
                   ThreadPool& thread_pool);
            void start(bool start_async);
            void shutdown();
    };
}

#endif
