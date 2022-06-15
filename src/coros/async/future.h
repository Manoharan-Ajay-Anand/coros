#ifndef COROS_ASYNC_FUTURE_H
#define COROS_ASYNC_FUTURE_H

#include <coroutine>

namespace coros {
    namespace async {
        struct Future {
            struct promise_type {
                Future get_return_object() { 
                    return {}; 
                }
                std::suspend_never initial_suspend() { return {}; }
                std::suspend_never final_suspend() noexcept { return {}; }
                void unhandled_exception() {}
                void return_void() {}
            };
        };
    }
}

#endif
