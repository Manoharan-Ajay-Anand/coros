#include "server/server.h"
#include "server/socket.h"

#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <string>

class TcpEchoServer : public server::ServerApplication {
    public:
        server::Future handle_socket(server::Socket* socket) {
            try {
                while (true) {
                    std::cerr << "Handling request" << std::endl;
                    std::string input;
                    char c = '\0';
                    while (c != '\n') {
                        co_await socket->read((uint8_t*) &c, 1);
                        input.push_back(c);
                    }
                    std::cerr << "Input: " << input << std::endl;
                    co_await socket->write((uint8_t*) input.c_str(), input.size());
                    co_await socket->flush();
                }
            } catch (std::runtime_error error) {
                std::cerr << "Error in coroutine: " << error.what() << std::endl;
                socket->close_socket();
            }
        }
};

int main() {
    TcpEchoServer tcp_echo_server;
    server::Server tcp_server(1025, tcp_echo_server);
    std::cerr << "Starting Server..." << std::endl;
    try {
        tcp_server.bootstrap();
    } catch (std::runtime_error error) {
        std::cerr << error.what() << std::endl;
    }
    return 0;
}
