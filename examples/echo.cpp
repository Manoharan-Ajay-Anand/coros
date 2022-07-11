#include "coros/server.h"
#include "coros/app.h"
#include "coros/socket.h"
#include "coros/async/future.h"

#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <cstdint>

class EchoApplication : public coros::ServerApplication {
    public:
        coros::async::AwaitableFuture get_input_string(coros::Socket* socket, std::string& input) {
            char c = (char) co_await socket->read_b();
            while (c != '\n') {
                if (c != '\r') {
                    input.push_back(c);
                }
                c = (char) co_await socket->read_b();
            }
        }

        coros::async::AwaitableFuture write_input_string(coros::Socket* socket, std::string& input) {
            const std::string newline = "\r\n";
            co_await socket->write((uint8_t*) input.data(), input.size());
            co_await socket->write((uint8_t*) newline.data(), newline.size());
            co_await socket->flush();
        }

        coros::async::Future on_request(std::shared_ptr<coros::Socket> socket) {
            try {
                const std::string close_cmd = "close";
                while (true) {
                    std::string input;
                    co_await get_input_string(socket.get(), input);
                    if (input.empty()) {
                        continue;
                    }
                    if (input == close_cmd) {
                        break;
                    }
                    co_await write_input_string(socket.get(), input);
                }
            } catch (std::runtime_error error) {
                std::cerr << "Error in coroutine: " << error.what() << std::endl;
            }
            socket->close_socket();
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
