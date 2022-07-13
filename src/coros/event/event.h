#ifndef COROS_EVENT_EVENT_H
#define COROS_EVENT_EVENT_H

#include <unordered_map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <poll.h>

namespace coros {
    namespace event {
        class SocketHandler {
            public:
                virtual void on_socket_event(bool can_read, bool can_write) = 0;
        };

        class SocketEventMonitor {
            private:
                bool is_shutdown;
                std::mutex event_mutex;
                std::vector<int> events;
                std::condition_variable event_condition;
                std::mutex handler_mutex;
                std::unordered_map<int, SocketHandler*> handler_map;
                void populate_events(std::vector<pollfd>& pollfds);
                void trigger_events(std::vector<pollfd>& pollfds);
            public:
                SocketEventMonitor();
                void register_socket(int socket_fd, SocketHandler& handler);
                void deregister_socket(int socket_fd);
                void listen_for_io(int socket_fd);
                void start();
                void shutdown();
        };
    }
}

#endif
