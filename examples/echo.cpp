#include "coros/server.h"
#include "coros/socket.h"
#include "coros/async/future.h"

#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <memory>

class EchoApplication : public coros::ServerApplication {
    public:
        coros::async::Future handle_socket(std::unique_ptr<coros::Socket> socket) {
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
};

int main() {
    EchoApplication echo_app;
    coros::Server echo_server(1025, echo_app);
    std::cerr << "Starting Server..." << std::endl;
    try {
        echo_server.setup();
        echo_server.start(true);
        std::getchar();
        echo_server.shutdown();
    } catch (std::runtime_error error) {
        std::cerr << error.what() << std::endl;
    }
    return 0;
}
