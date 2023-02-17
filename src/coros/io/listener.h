#ifndef COROS_IO_LISTENER_H
#define COROS_IO_LISTENER_H

#include "read_awaiter.h"
#include "write_awaiter.h"

#include <coroutine>
#include <optional>
#include <mutex>

namespace coros::base {
    class IoEventMonitor;

    class ThreadPool;

    class IoEventListener {
        private:
            ThreadPool& thread_pool;
            int epoll_fd;
            int io_fd;
            std::mutex listener_mutex;
            bool listening_read;
            bool listening_write;
            std::optional<std::coroutine_handle<>> read_handle_opt;
            std::optional<std::coroutine_handle<>> write_handle_opt;
            void listen_for_event(bool need_read, bool need_write);
            bool run_handle(std::optional<std::coroutine_handle<>>& handle_opt, bool has_event);
        public:
            IoEventListener(ThreadPool& thread_pool, int epoll_fd, int io_fd);
            ~IoEventListener();
            void on_event(bool can_read, bool can_write);
            void set_read_handle(std::coroutine_handle<> read_handle);
            void set_write_handle(std::coroutine_handle<> write_handle);
            IoReadAwaiter await_read();
            IoWriteAwaiter await_write();
    };
}

#endif
