#ifndef COROS_EVENT_MONITOR_H
#define COROS_EVENT_MONITOR_H

#include <atomic>
#include <mutex>
#include <sys/epoll.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define POLL_MAX_EVENTS 1000
#define POLL_TIMEOUT 2000

namespace coros {
    namespace event {
        struct SocketEventHandler;
        
        class SocketEventMonitor {
            private:
                std::atomic_bool is_shutdown;
                int epoll_fd;
                std::mutex handler_mutex;
                std::unordered_map<int, SocketEventHandler*> handler_map;
                void trigger_events(std::vector<epoll_event>& events, int count);
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
