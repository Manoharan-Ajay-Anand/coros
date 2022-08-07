#include "app.h"
#include "socket.h"

#include <memory>
#include <mutex>
#include <unordered_map>

void coros::ServerApplication::handle_socket(Server& server, std::shared_ptr<Socket> socket_ptr) {
    {
        std::lock_guard<std::mutex> socket_lock(socket_mutex);
        socket_map[socket_ptr->get_fd()] = socket_ptr;
    }
    on_request(server, std::move(socket_ptr));
}

void coros::ServerApplication::shutdown() {
    std::lock_guard<std::mutex> socket_lock(socket_mutex);
    for (auto it = socket_map.begin(); it != socket_map.end(); ++it) {
        if (std::shared_ptr<Socket> socket = it->second.lock()) {
            socket->close_socket();
        }
    }
}

