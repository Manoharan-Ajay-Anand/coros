#include "socket.h"
#include "server.h"
#include "concurrent/thread_pool.h"
#include "concurrent/coroutine.h"
#include "channel/socket_input.h"
#include "channel/socket_output.h"

#include <cstdint>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <coroutine>

server::Socket::Socket(int socket_fd, sockaddr_storage client_addr, socklen_t addr_size, 
    Server& server, event::SocketEventMonitor& event_monitor, concurrent::ThreadPool& thread_pool) 
: event_monitor(event_monitor), server(server), thread_pool(thread_pool),
  input_channel(*this), output_channel(*this) {
    this->socket_fd = socket_fd;
    this->client_addr = client_addr;
    this->addr_size = addr_size;
    this->read_handler_set = false;
    this->write_handler_set = false;
}

server::Socket::~Socket() {
    shutdown(socket_fd, SHUT_RDWR);
}

int server::Socket::get_fd() {
    return socket_fd;
}

void server::Socket::on_socket_event(bool can_read, bool can_write) {
    if (!can_read && read_handler_set) {
        event_monitor.listen_for_event(socket_fd, true, false);
    }
    if (!can_write && write_handler_set) {
        event_monitor.listen_for_event(socket_fd, false, true);
    }
    if (can_read && read_handler_set) {
        read_handler_set = false;
        thread_pool.execute([&]() { read_handler(); });
    }
    if (can_write && write_handler_set) {
        write_handler_set = false;
        thread_pool.execute([&]() { write_handler(); });
    }
}

void server::Socket::set_read_handler(std::function<void()> read_handler) {
    if (read_handler_set) {
        throw std::string("Read handler already set");
    }
    this->read_handler = read_handler;
    this->read_handler_set = true;
    event_monitor.listen_for_event(socket_fd, true, false);
}
            
void server::Socket::set_write_handler(std::function<void()> write_handler) {
    if (write_handler_set) {
        throw std::string("Write handler already set");
    }
    this->write_handler = write_handler;
    this->write_handler_set = true;
    event_monitor.listen_for_event(socket_fd, false, true);
}

concurrent::Future server::Socket::handle_request() {
    event_monitor.register_handler(socket_fd, this);
    while (true) {
        std::cerr << "Handling request" << std::endl;
        std::string input;
        char c = '\0';
        while (c != '\n') {
            co_await input_channel.read_to_mem((uint8_t *) &c, 1);
            input.push_back(c);
        }
        std::cout << input;
        co_await output_channel.write_from_mem((uint8_t *) input.c_str(), input.length());
        co_await output_channel.flush();
    }
}
