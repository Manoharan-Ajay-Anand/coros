add_library(coros STATIC
    src/coros/async/thread_pool.cpp
    src/coros/commons/error.cpp
    src/coros/io/listener.cpp
    src/coros/io/monitor.cpp
    src/coros/io/read_awaiter.cpp
    src/coros/io/write_awaiter.cpp
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
