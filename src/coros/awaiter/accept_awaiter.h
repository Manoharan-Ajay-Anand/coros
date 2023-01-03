#ifndef COROS_AWAITER_ACCEPT_AWAITER_H
#define COROS_AWAITER_ACCEPT_AWAITER_H

#include "coros/network/socket.h"

#include <cstddef>
#include <coroutine>
#include <memory>
#include <optional>
#include <stdexcept>
#include <exception>

namespace coros::base {
    class ThreadPool;

    class SocketEventManager;

    class SocketEventMonitor;

    struct SocketAcceptAwaiter {
        const int server_fd;
        ThreadPool& thread_pool;
        SocketEventManager& event_manager;
        SocketEventMonitor& event_monitor;
        std::optional<std::runtime_error> error_optional;
        SocketDetails details;
        SocketAcceptAwaiter(int server_fd, ThreadPool& thread_pool, 
                            SocketEventManager& event_manager, SocketEventMonitor& monitor);
        SocketOperation attempt_accept();
        void accept_socket(std::coroutine_handle<> handle);
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        std::shared_ptr<Socket> await_resume();
    };
}
#endif
