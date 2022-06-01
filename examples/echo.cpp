#include "coros/server.h"
#include "coros/socket.h"
#include "coros/concurrent/thread_pool.h"
#include "coros/event/event.h"

#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <string>

class EchoApplication : public coros::ServerApplication {
    public:
        coros::Future handle_socket(coros::Socket* socket) {
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
    coros::concurrent::ThreadPool thread_pool;
    coros::event::SocketEventMonitor event_monitor;
    EchoApplication echo_app;
    coros::Server echo_server(1025, echo_app, event_monitor, thread_pool);
    std::cerr << "Starting Server..." << std::endl;
    try {
        echo_server.bootstrap();
        getchar();
        echo_server.shutdown();
        thread_pool.shutdown();
    } catch (std::runtime_error error) {
        std::cerr << error.what() << std::endl;
    }
    return 0;
}
