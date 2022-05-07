#ifndef CONCURRENT_COROUTINE_H
#define CONCURRENT_COROUTINE_H

#include <coroutine>
#include <functional>

namespace server {
    namespace concurrent {
        struct Future {
            struct promise_type {
                Future get_return_object() { 
                    return { std::coroutine_handle<promise_type>::from_promise(*this) }; 
                }
                std::suspend_never initial_suspend() { return {}; }
                std::suspend_always final_suspend() noexcept { return {}; }
                void unhandled_exception() {}
                void return_void() {}
            };
            std::coroutine_handle<promise_type> handle;
        };
    }
}

#endif
