#ifndef COROS_APP_H
#define COROS_APP_H

#include "async/future.h"

#include <memory>
#include <unordered_map>
#include <mutex>

namespace coros {
    class Socket;

    class ServerApplication {
        private:
            std::mutex socket_mutex;
            std::unordered_map<int, std::weak_ptr<Socket>> socket_map;
        public:
            void handle_socket(std::shared_ptr<Socket> socket_ptr);
            virtual async::Future on_request(std::shared_ptr<Socket> socket) = 0;
            void shutdown();
    };
}

#endif
