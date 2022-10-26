#ifndef COROS_EVENT_EXECUTOR_H
#define COROS_EVENT_EXECUTOR_H

#include <mutex>
#include <functional>
#include <optional>

namespace coros {
    namespace async {
        class ThreadPool;
    }

    namespace event {
        class EventHandlerExecutor {
            private:
                std::mutex handler_mutex;
                std::optional<std::function<void()>> handler_optional;
                async::ThreadPool& thread_pool;
            public:
                EventHandlerExecutor(async::ThreadPool& thread_pool);
                void on_event();
                void set_handler(std::function<void()> handler);
                bool has_handler();
        };
    }
}

#endif
