#ifndef COROS_APP_H
#define COROS_APP_H

#include "async/future.h"

#include <memory>
#include <unordered_map>
#include <mutex>

namespace coros::network {
    class Socket;

    class Server;
}

namespace coros {
    class ServerApplication {
        private:
            std::mutex socket_mutex;
            std::unordered_map<int, std::weak_ptr<network::Socket>> socket_map;
        public:
            void handle_socket(network::Server& server, std::shared_ptr<network::Socket> socket_ptr);
            virtual async::Future on_request(network::Server& server, std::shared_ptr<network::Socket> socket) = 0;
            void shutdown();
    };
}

#endif
