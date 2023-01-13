#include "server.h"
#include "socket.h"
#include "coros/app.h"
#include "coros/async/thread_pool.h"
#include "coros/event/monitor.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>

coros::base::Server::Server(short port, ServerApplication& server_app,
                            SocketEventMonitor& event_monitor, ThreadPool& thread_pool) 
        : server_socket(port, thread_pool, event_monitor), server_app(server_app),
          event_monitor(event_monitor), thread_pool(thread_pool) {
    this->marked_for_close = false;
}

coros::base::Future coros::base::Server::run_server_loop() {
    try {
        while (!marked_for_close) {
            std::shared_ptr<Socket> socket = co_await server_socket.accept();
            thread_pool.run([&, socket_ptr = std::move(socket)] {
                server_app.handle_socket(
                    *this,
                    std::move(socket_ptr)
                );
            });
        }
    } catch (std::runtime_error error) {
        std::cout << "Error: " << error.what() << std::endl;
    }
    server_socket.close_socket();
}

void coros::base::Server::start(bool start_async) {
    thread_pool.run([&] {
        run_server_loop();
    });
    if (start_async) {
        return thread_pool.run([&] { event_monitor.start(); });
    }
    event_monitor.start();
}

void coros::base::Server::shutdown() {
    if (marked_for_close.exchange(true)) {
        throw std::runtime_error(std::string("Server shutdown(): Server already shutdown"));
    }
    server_app.shutdown();
    event_monitor.shutdown();
    thread_pool.shutdown();
}
