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
                                                        event_monitor(monitor) {
    this->server_fd = server_fd;
}

coros::base::SocketOperation coros::base::SocketAcceptAwaiter::attempt_accept() {
    try {
        details.socket_fd = accept(server_fd, (sockaddr*) &details.client_addr, &details.addr_size);
        if (details.socket_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return SOCKET_OP_BLOCK;
            }
            throw std::runtime_error(std::string("Socket accept(): ").append(strerror(errno)));
        }
        set_non_blocking_socket(details.socket_fd);
        return SOCKET_OP_SUCCESS;
    } catch (std::runtime_error e) {
        error = e;
        return SOCKET_OP_ERROR;
    }
}

void coros::base::SocketAcceptAwaiter::accept_socket(std::coroutine_handle<> handle) {
    SocketOperation operation = attempt_accept();
    if (operation == SOCKET_OP_BLOCK) {
        return event_manager.set_read_handler([&, handle]() {
            accept_socket(handle);
        });
    }
    handle.resume();
}

bool coros::base::SocketAcceptAwaiter::await_ready() noexcept {
    SocketOperation operation = attempt_accept();
    return operation != SOCKET_OP_BLOCK;
}

void coros::base::SocketAcceptAwaiter::await_suspend(std::coroutine_handle<> handle) {
    event_manager.set_read_handler([&, handle]() {
        accept_socket(handle);
    });
}

std::shared_ptr<coros::base::Socket> coros::base::SocketAcceptAwaiter::await_resume() {
    if (error) {
        throw error.value();
    }
    return std::make_shared<Socket>(details, event_monitor, thread_pool);
}
