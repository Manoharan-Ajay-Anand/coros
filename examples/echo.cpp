#include "coros/network/server.h"
#include "coros/app.h"
#include "coros/network/socket.h"
#include "coros/event/monitor.h"
#include "coros/async/thread_pool.h"
#include "coros/async/future.h"

#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <cstddef>

class EchoApplication : public coros::ServerApplication {
    private:
        std::string newline = "\r\n";

        coros::async::AwaitableValue<std::string> get_input(coros::network::Socket* socket) {
            std::string input;
            char c = static_cast<char>(co_await socket->read_b());
            while (c != '\n') {
                if (c != '\r') {
                    input.push_back(c);
                }
                c = static_cast<char>(co_await socket->read_b());
            }
            co_return std::move(input);
        }

        coros::async::AwaitableFuture echo(coros::network::Socket* socket, std::string& input) {
            co_await socket->write(reinterpret_cast<std::byte*>(input.data()), input.size());
            co_await socket->write(reinterpret_cast<std::byte*>(newline.data()), 
                                   newline.size());
            co_await socket->flush();
        }

    public:
        coros::async::Future on_request(coros::network::Server& server, 
                                        std::shared_ptr<coros::network::Socket> socket) {
            try {
                const std::string close_cmd = "close";
                while (true) {
                    std::string input = co_await get_input(socket.get());
                    if (input.empty()) {
                        continue;
                    }
                    if (input == close_cmd) {
                        break;
                    }
                    co_await echo(socket.get(), input);
                }
            } catch (std::runtime_error error) {
                std::cerr << "Error in coroutine: " << error.what() << std::endl;
            }
            socket->close_socket();
        }
};


void start_server(coros::network::Server& server) {
    try {
        server.setup();
        server.start(true);
        std::getchar();
        server.shutdown();
    } catch (std::runtime_error error) {
        std::cerr << error.what() << std::endl;
    }
}

int main() {
    coros::async::ThreadPool thread_pool;
    coros::event::SocketEventMonitor event_monitor;
    EchoApplication echo_app;
    coros::network::Server echo_server(1025, echo_app, event_monitor, thread_pool);
    std::cout << "Starting Server..." << std::endl;
    start_server(echo_server);
    return 0;
}
