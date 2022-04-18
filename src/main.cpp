#include "server/server.h"

#include <iostream>
#include <stdexcept>

int main() {
    server::Server tcp_server(1025);
    std::cerr << "Starting Server..." << std::endl;
    try {
        tcp_server.bootstrap();
    } catch (std::runtime_error error) {
        std::cerr << error.what() << std::endl;
    }
    return 0;
}
