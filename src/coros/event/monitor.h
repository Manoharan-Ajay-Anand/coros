#ifndef COROS_EVENT_MONITOR_H
#define COROS_EVENT_MONITOR_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <poll.h>

#define POLL_TIMEOUT 2000

namespace coros {
    namespace event {
        struct SocketEventHandler;
        
        class SocketEventMonitor {
            private:
                bool is_shutdown;
                std::mutex event_mutex;
                std::unordered_set<int> event_set;
                std::condition_variable event_condition;
                std::mutex handler_mutex;
                std::unordered_map<int, SocketEventHandler*> handler_map;
                void populate_events(std::vector<pollfd>& pollfds);
                void trigger_events(std::vector<pollfd>& pollfds);
            public:
                SocketEventMonitor();
                void register_socket(int socket_fd, SocketEventHandler& handler);
                void deregister_socket(int socket_fd);
                void listen_for_io(int socket_fd);
                void start();
                void shutdown();
        };
    }
}

#endif
