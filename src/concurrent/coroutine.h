#ifndef CONCURRENT_COROUTINE_H
#define CONCURRENT_COROUTINE_H

#include <coroutine>
#include <functional>

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
    struct Awaiter {
        std::function<void(std::coroutine_handle<>)> func;
        constexpr bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<> handle) { func(handle); }
        constexpr void await_resume() const noexcept {}
    };
}

#endif
