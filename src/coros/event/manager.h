#ifndef COROS_EVENT_MANAGER_H
#define COROS_EVENT_MANAGER_H

#include "handler.h"
#include "executor.h"

#include <atomic>
#include <functional>
#include <mutex>

namespace coros::base {
    class ThreadPool;

    class SocketEventMonitor;

    class SocketEventManager : public SocketEventHandler {
        private:
            int socket_fd;
            SocketEventMonitor& event_monitor;
            std::mutex manager_mutex;
            bool is_registered;
            bool marked_for_close;
            std::atomic_bool waiting_for_io;
            EventHandlerExecutor read_executor;
            EventHandlerExecutor write_executor;
            void listen_for_io();
        public:
            SocketEventManager(SocketEventMonitor& event_monitor, ThreadPool& thread_pool);
            void register_socket_fd(int socket_fd);
            void on_socket_event(bool can_read, bool can_write);
            void set_read_handler(std::function<void()> handler);
            void set_write_handler(std::function<void()> handler);
            void close();
    };
}

#endif
