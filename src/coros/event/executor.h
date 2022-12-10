#ifndef COROS_EVENT_EXECUTOR_H
#define COROS_EVENT_EXECUTOR_H

#include <mutex>
#include <functional>
#include <optional>

namespace coros::base {
    class ThreadPool;

    class EventHandlerExecutor {
        private:
            std::mutex handler_mutex;
            std::optional<std::function<void()>> handler_optional;
            ThreadPool& thread_pool;
        public:
            EventHandlerExecutor(ThreadPool& thread_pool);
            void on_event();
            void set_handler(std::function<void()> handler);
            bool has_handler();
    };
}

#endif
