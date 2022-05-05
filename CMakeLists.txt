cmake_minimum_required(VERSION 3.1)

project(Server
    VERSION 1.0
    DESCRIPTION "Server made in C++"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")

add_executable(Server
    src/concurrent/thread_pool.cpp
    src/event/event.cpp
    src/memory/read_socket_buffer.cpp
    src/memory/write_socket_buffer.cpp
    src/memory/socket_buffer.cpp
    src/server/server.cpp
    src/socket/socket.cpp
    src/socket/awaiter.cpp
    src/main.cpp)

find_package(Threads REQUIRED)

target_include_directories(Server PRIVATE src/)

target_link_libraries(Server PRIVATE Threads::Threads)
