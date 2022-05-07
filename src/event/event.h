#ifndef EVENT_EVENT_H
#define EVENT_EVENT_H

#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <poll.h>

namespace server {
    namespace event {
        struct SocketEvent {
            int socket_fd;
            bool do_read;
            bool do_write;
        };
        
        class SocketHandler {
            public:
                virtual void on_socket_event(bool can_read, bool can_write) = 0;
        };

        class SocketEventMonitor {
            private:
                std::mutex event_mutex;
                std::queue<SocketEvent> event_queue;
                std::condition_variable events_available;
                std::mutex handler_mutex;
                std::unordered_map<int, SocketHandler*> handler_map;
                void populate_events(std::vector<pollfd>& pollfds);
                void trigger_events(std::vector<pollfd>& pollfds);
            public:
                void register_socket(int socket_fd, SocketHandler* handler);
                void deregister_socket(int socket_fd);
                void listen_for_read(int socket_fd);
                void listen_for_write(int socket_fd);
                void start();
        };
    }
}

#endif
