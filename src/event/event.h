#ifndef EVENT_EVENT_H
#define EVENT_EVENT_H

#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <poll.h>

namespace event {
    struct SocketEvent {
        int socket_fd;
        bool do_read;
        bool do_write;
    };
    class SocketEventHandler {
        public:
            virtual void on_socket_event(bool can_read, bool can_write) = 0;
    };
    class SocketEventMonitor {
        private:
            std::mutex handler_mutex;
            std::unordered_map<int, SocketEventHandler*> handler_map;
            std::mutex event_mutex;
            std::queue<SocketEvent> event_queue;
            std::condition_variable events_available;
            void set_event(SocketEvent& socket_event, pollfd& pollfd);
            void populate_events(std::vector<pollfd>& pollfds);
            void trigger_events(std::vector<pollfd>& pollfds);
        public:
            void register_handler(int socket_fd, SocketEventHandler* handler);
            void deregister_handler(int socket_fd);
            void listen_for_event(int socket_fd, bool do_read, bool do_write);
            void start();
    };
}

#endif
