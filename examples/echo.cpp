#include "coros/server.h"
#include "coros/socket.h"
#include "coros/async/future.h"

#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <memory>
#include <mutex>
#include <vector>

class EchoApplication : public coros::ServerApplication {
    private:
        std::mutex sockets_mutex;
        std::vector<std::unique_ptr<coros::Socket>> sockets;
    public:
        coros::async::Future handle_socket(std::unique_ptr<coros::Socket> socket_ptr) {
            coros::Socket* socket = socket_ptr.get();
            {
                std::lock_guard<std::mutex> sockets_lock(sockets_mutex);
                sockets.push_back(std::move(socket_ptr));
            }
            try {
                const std::string newline = "\r\n";
                const std::string close_cmd = "close";
                while (true) {
                    std::string input;
                    char c = (char) co_await socket->read_b();
                    while (c != '\n') {
                        if (c != '\r') {
                            input.push_back(c);
                        }
                        c = (char) co_await socket->read_b();
                    }
                    if (input.empty()) {
                        continue;
                    }
                    if (input == close_cmd) {
                        break;
                    }
                    co_await socket->write((uint8_t*) input.data(), input.size());
                    co_await socket->write((uint8_t*) newline.data(), newline.size());
                    co_await socket->flush();
                }
            } catch (std::runtime_error error) {
                std::cerr << "Error in coroutine: " << error.what() << std::endl;
            }
            socket->close_socket();
        }

        void shutdown() {
            sockets.clear();
        }
};


void start_server(coros::Server& server) {
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
    EchoApplication echo_app;
    coros::Server echo_server(1025, echo_app);
    std::cout << "Starting Server..." << std::endl;
    start_server(echo_server);
    return 0;
}
