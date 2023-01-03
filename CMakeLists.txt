cmake_minimum_required(VERSION 3.1)

project(Coros
    VERSION 1.0
    DESCRIPTION "Server based on C++ 20 coroutines"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")

add_library(coros STATIC
    src/coros/async/thread_pool.cpp
    src/coros/awaiter/accept_awaiter.cpp
    src/coros/awaiter/flush_awaiter.cpp
    src/coros/awaiter/read_awaiter.cpp
    src/coros/awaiter/skip_awaiter.cpp
    src/coros/awaiter/write_awaiter.cpp
    src/coros/event/executor.cpp
    src/coros/event/manager.cpp
    src/coros/event/monitor.cpp
    src/coros/memory/buffer.cpp
    src/coros/network/server.cpp
    src/coros/network/socket.cpp
    src/coros/network/server_socket.cpp
    src/coros/network/stream.cpp
    src/coros/network/util.cpp
    src/coros/app.cpp)

find_package(Threads REQUIRED)
target_include_directories(coros PUBLIC src)
target_link_libraries(coros PRIVATE Threads::Threads)

add_executable(echo EXCLUDE_FROM_ALL examples/echo.cpp)
target_link_libraries(echo PRIVATE coros)
