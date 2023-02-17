#ifndef COROS_SERVER_H
#define COROS_SERVER_H

#include "coros/async/future.h"
#include "coros/network/server_socket.h"

#include <atomic>
#include <string>

namespace coros::base {
    class ServerApplication;

    class ThreadPool;

    class IoEventMonitor;

    class Server {
        private:
            ServerApplication& server_app;
            IoEventMonitor& io_monitor;
            ThreadPool& thread_pool;
            ServerSocket server_socket;
            std::atomic_bool marked_for_close;
            Future run_server_loop();
        public:
            Server(short port, ServerApplication& server_app, IoEventMonitor& io_monitor, 
                   ThreadPool& thread_pool);
            void start();
            void shutdown();
    };
}

#endif
