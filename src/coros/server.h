#ifndef COROS_SERVER_H
#define COROS_SERVER_H

#include "event/event.h"

#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include <netdb.h>
#include <coroutine>

namespace coros {
    namespace concurrent {
        class ThreadPool;
    }

    class Socket;

    struct Future {
            struct promise_type {
                Future get_return_object() { 
                    return { std::coroutine_handle<promise_type>::from_promise(*this) }; 
                }
                std::suspend_never initial_suspend() { return {}; }
                std::suspend_never final_suspend() noexcept { return {}; }
                void unhandled_exception() {}
                void return_void() {}
            };
            std::coroutine_handle<promise_type> handle;
        };
    
    class ServerApplication {
        public:
            virtual Future handle_socket(Socket* socket) = 0;
    };
    
    class Server : public event::SocketHandler {
        private:
            ServerApplication& server_app;
            event::SocketEventMonitor& event_monitor;
            concurrent::ThreadPool& thread_pool;
            std::mutex socket_mutex;
            std::unordered_map<int, std::unique_ptr<Socket>> socket_map;
            std::string service;
            int server_socketfd;
            addrinfo get_local_addr_info();
            void set_non_blocking(int socket_fd);
        public:
            Server(short port, ServerApplication& server_app, 
                event::SocketEventMonitor& event_monitor, concurrent::ThreadPool& thread_pool);
            void on_socket_event(bool can_read, bool can_write);
            void destroy_socket(int socket_fd);
            void bootstrap();
            void shutdown();
    };
}

#endif
