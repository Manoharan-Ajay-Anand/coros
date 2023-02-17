#ifndef COROS_IO_MONITOR_H
#define COROS_IO_MONITOR_H

#include "listener.h"

#include <atomic>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <sys/epoll.h>

#define IO_POLL_MAX_EVENTS 1000
#define IO_POLL_TIMEOUT 2000

namespace coros::base {
    class IoEventListener;

    class ThreadPool;

    class IoEventMonitor {
        private:
            ThreadPool& thread_pool;
            std::atomic_bool is_shutdown;
            std::mutex fd_mutex;
            std::unordered_map<int, std::unique_ptr<IoEventListener>> fd_listener_map;
            int epoll_fd;
            void run();
            void trigger_events(epoll_event* events, int count);
        public:
            IoEventMonitor(ThreadPool& thread_pool);
            void shutdown();
            IoEventListener* register_fd(int fd);
            void remove_fd(int fd);
    };
}

#endif
