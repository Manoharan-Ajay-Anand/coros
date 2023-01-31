#include "accept_awaiter.h"

#include "coros/network/socket.h"
#include "coros/network/util.h"

#include <iostream>
#include <cstddef>
#include <coroutine>
#include <memory>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>

coros::base::SocketAcceptAwaiter::SocketAcceptAwaiter(int server_fd, ThreadPool& thread_pool, 
                                                      SocketEventManager& event_manager, 
                                                      SocketEventMonitor& monitor)
                                                      : thread_pool(thread_pool), 
                                                        event_manager(event_manager), 
                                                        event_monitor(monitor),
                                                        server_fd(server_fd) {
}

void coros::base::SocketAcceptAwaiter::accept_socket(std::coroutine_handle<> handle) {
    try {
        details.socket_fd = accept(server_fd, (sockaddr*) &details.client_addr, &details.addr_size);
        if (details.socket_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return event_manager.set_read_handler([&, handle]() {
                    accept_socket(handle);
                });
            }
            throw std::runtime_error(std::string("Socket accept(): ").append(strerror(errno)));
        }
        set_non_blocking_socket(details.socket_fd);
    } catch (std::runtime_error error) {
        error_optional = error;
    }
    handle.resume();
}

bool coros::base::SocketAcceptAwaiter::await_ready() noexcept {
    return false;
}

void coros::base::SocketAcceptAwaiter::await_suspend(std::coroutine_handle<> handle) {
    accept_socket(handle);
}

std::shared_ptr<coros::base::Socket> coros::base::SocketAcceptAwaiter::await_resume() {
    if (error_optional) {
        throw error_optional.value();
    }
    return std::make_shared<Socket>(details, event_monitor, thread_pool);
}
