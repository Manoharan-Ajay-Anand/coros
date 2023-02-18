#include "socket.h"

#include "coros/io/monitor.h"
#include "coros/io/listener.h"

#include <atomic>
#include <algorithm>
#include <cstddef>
#include <sys/socket.h>
#include <unistd.h>

#define SOCKET_BUFFER_SIZE 8192

coros::base::Socket::Socket(SocketDetails details, IoEventMonitor& io_monitor) 
        : details(details), io_monitor(io_monitor), 
          stream(details.socket_fd), input_buffer(SOCKET_BUFFER_SIZE), 
          output_buffer(SOCKET_BUFFER_SIZE), is_closed(false) {
    io_listener = io_monitor.register_fd(details.socket_fd);
}

void coros::base::Socket::read_available(std::byte*& dest, long long& size, long long& total_read) {
    long long buffer_remaining = input_buffer.get_total_remaining();
    if (buffer_remaining == 0) {
        return;
    }
    long long size_to_read = std::min(size, buffer_remaining);
    input_buffer.read(dest, size_to_read);
    dest += size_to_read;
    size -= size_to_read;
    total_read += size_to_read;
}

coros::base::AwaitableValue<long long> coros::base::Socket::read(std::byte* dest, 
                                                                 long long size,
                                                                 bool read_fully) {
    long long total_read = 0;
    read_available(dest, size, total_read);
    while (size > 0) {
        SocketOperation status = stream.recv_from_socket(input_buffer);
        read_available(dest, size, total_read);
        if (size == 0 || status == SOCKET_OP_SUCCESS) {
            continue;
        }
        if (status == SOCKET_OP_BLOCK) {
            co_await io_listener->await_read();
        } else if (status == SOCKET_OP_CLOSE) {
            if (read_fully) {
                throw std::runtime_error("Socket read(): Socket has been closed");
            }
            break;
        }
    }
    co_return total_read;
}

void coros::base::Socket::skip_available(long long& size, long long& total_skipped) {
    long long buffer_remaining = input_buffer.get_total_remaining();
    if (buffer_remaining == 0) {
        return;
    }
    long long size_to_skip = std::min(size, buffer_remaining);
    input_buffer.increment_read_pointer(size_to_skip);
    size -= size_to_skip;
    total_skipped += size_to_skip;
}

coros::base::AwaitableValue<long long> coros::base::Socket::skip(long long size, bool skip_fully) {
    long long total_skipped = 0;
    skip_available(size, total_skipped);
    while (size > 0) {
        SocketOperation status = stream.recv_from_socket(input_buffer);
        skip_available(size, total_skipped);
        if (size == 0 || status == SOCKET_OP_SUCCESS) {
            continue;
        }
        if (status == SOCKET_OP_BLOCK) {
            co_await io_listener->await_read();
        } else if (status == SOCKET_OP_CLOSE) {
            if (skip_fully) {
                throw std::runtime_error("Socket skip(): Socket has been closed");
            }
            break;
        }
    }
    co_return total_skipped;
}

void coros::base::Socket::write_available(std::byte*& src, long long& size) {
    long long buffer_capacity = output_buffer.get_total_capacity();
    if (buffer_capacity == 0) {
        return;
    }
    long long size_to_write = std::min(buffer_capacity, size);
    output_buffer.write(src, size_to_write);
    src += size_to_write;
    size -= size_to_write;
}

coros::base::AwaitableFuture coros::base::Socket::write(std::byte* src, long long size) {
    write_available(src, size);
    while (size > 0) {
        SocketOperation status = stream.send_to_socket(output_buffer);
        write_available(src, size);
        if (size == 0 || status == SOCKET_OP_SUCCESS) {
            continue;
        }
        if (status == SOCKET_OP_BLOCK) {
            co_await io_listener->await_write();
        } else if (status == SOCKET_OP_CLOSE) {
            throw std::runtime_error("Socket write(): Socket has been closed");
        }
    }
}

coros::base::AwaitableFuture coros::base::Socket::flush() {
    while (output_buffer.get_total_remaining() > 0) {
        SocketOperation status = stream.send_to_socket(output_buffer);
        if (status == SOCKET_OP_BLOCK) {
            co_await io_listener->await_write();
        } else if (status == SOCKET_OP_CLOSE) {
            throw std::runtime_error("Socket flush(): Socket has been closed");
        }
    }
}

int coros::base::Socket::get_fd() {
    return details.socket_fd;
}

void coros::base::Socket::close_socket() {
    if (is_closed.exchange(true)) {
        return;
    }
    stream.close();
    io_listener->close();
    io_monitor.remove_fd(details.socket_fd);
    close(details.socket_fd);
}
