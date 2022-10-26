cmake_minimum_required(VERSION 3.1)

project(Coros
    VERSION 1.0
    DESCRIPTION "Server based on C++ 20 coroutines"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")

add_library(coros STATIC
    src/coros/async/read_awaiter.cpp
    src/coros/async/write_awaiter.cpp
    src/coros/async/thread_pool.cpp
    src/coros/event/executor.cpp
    src/coros/event/manager.cpp
    src/coros/event/monitor.cpp
    src/coros/memory/read_socket_buffer.cpp
    src/coros/memory/write_socket_buffer.cpp
    src/coros/memory/socket_buffer.cpp
    src/coros/app.cpp
    src/coros/server.cpp
    src/coros/socket.cpp)

find_package(Threads REQUIRED)
target_include_directories(coros PUBLIC src/)
target_link_libraries(coros PRIVATE Threads::Threads)

add_executable(echo examples/echo.cpp)
target_link_libraries(echo PRIVATE coros)
