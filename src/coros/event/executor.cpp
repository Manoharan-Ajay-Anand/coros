#include "executor.h"
#include "coros/async/thread_pool.h"

#include <mutex>

coros::event::EventHandlerExecutor::EventHandlerExecutor(coros::async::ThreadPool& thread_pool)
        : thread_pool(thread_pool) {
}

void coros::event::EventHandlerExecutor::on_event() {
    std::lock_guard<std::mutex> guard(handler_mutex);
    if (handler_optional) {
        thread_pool.run(handler_optional.value());
        handler_optional.reset();
    }
}

void coros::event::EventHandlerExecutor::set_handler(std::function<void()> handler) {
    std::lock_guard<std::mutex> guard(handler_mutex);
    if (handler_optional) {
        throw std::runtime_error("EventHandlerExecutor set_handler error: Handler already set");
    }
    handler_optional = handler;
}

bool coros::event::EventHandlerExecutor::has_handler() {
    return handler_optional.has_value();
}