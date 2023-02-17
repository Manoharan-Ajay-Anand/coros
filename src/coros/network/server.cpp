#include "server.h"
#include "socket.h"
#include "coros/app.h"
#include "coros/async/thread_pool.h"
#include "coros/io/monitor.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>

coros::base::Server::Server(short port, ServerApplication& server_app,
                            IoEventMonitor& io_monitor, ThreadPool& thread_pool) 
        : server_socket(port, io_monitor), server_app(server_app),
          io_monitor(io_monitor), thread_pool(thread_pool) {
    this->marked_for_close = false;
}

coros::base::Future coros::base::Server::run_server_loop() {
    try {
        while (!marked_for_close) {
            std::shared_ptr<Socket> socket = co_await server_socket.accept_conn();
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

void coros::base::Server::start() {
    thread_pool.run([&] {
        run_server_loop();
    });
}

void coros::base::Server::shutdown() {
    if (marked_for_close.exchange(true)) {
        throw std::runtime_error(std::string("Server shutdown(): Server already shutdown"));
    }
    server_app.shutdown();
    io_monitor.shutdown();
    thread_pool.shutdown();
}
