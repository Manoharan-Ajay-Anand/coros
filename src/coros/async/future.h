#ifndef COROS_ASYNC_FUTURE_H
#define COROS_ASYNC_FUTURE_H

#include <coroutine>
#include <exception>

namespace coros {
    namespace async {
        struct Future {
            struct promise_type {
                Future get_return_object() { return {}; }
                std::suspend_never initial_suspend() { return {}; }
                std::suspend_never final_suspend() noexcept { return {}; }
                void unhandled_exception() {}
                void return_void() {}
            };
        };

        struct CoroAwaiter {
            std::coroutine_handle<> waiting;

            inline bool await_ready() noexcept {
                return false;
            }
            
            void await_suspend(std::coroutine_handle<> handle) noexcept {
                if (waiting) {
                    waiting.resume();
                }
            }
            
            void await_resume() noexcept {
            }
        };

        struct AwaitableFuture {
            struct promise_type {
                std::coroutine_handle<> waiting;
                bool has_error = false;
                std::exception_ptr exception;

                AwaitableFuture get_return_object() { 
                    return { std::coroutine_handle<promise_type>::from_promise(*this) }; 
                }
                
                std::suspend_never initial_suspend() { return {}; }
                
                CoroAwaiter final_suspend() noexcept {
                    return { waiting };
                }
                
                void unhandled_exception() {
                    has_error = true;
                    exception = std::current_exception(); 
                }
                
                void return_void() {}
            };

            std::coroutine_handle<promise_type> coro_handle;
            
            bool await_ready() noexcept {
                return coro_handle.done();
            }
            
            void await_suspend(std::coroutine_handle<> handle) {
                coro_handle.promise().waiting = handle;
            }
            
            void await_resume() {
                bool has_error = coro_handle.promise().has_error;
                std::exception_ptr exception = coro_handle.promise().exception;
                coro_handle.destroy();
                if (has_error) {
                    std::rethrow_exception(exception);
                }
            }
        };

        template<typename T>
        struct AwaitableValue {
            struct promise_type {
                T val {};
                std::coroutine_handle<> waiting;
                bool has_error = false;
                std::exception_ptr exception;

                AwaitableValue<T> get_return_object() { 
                    return { std::coroutine_handle<promise_type>::from_promise(*this) }; 
                }
                
                std::suspend_never initial_suspend() { return {}; }
                
                CoroAwaiter final_suspend() noexcept {
                    return { waiting };
                }
                
                void unhandled_exception() {
                    has_error = true;
                    exception = std::current_exception(); 
                }
                
                void return_value(const T& val) {
                    this->val = val;
                }

                void return_value(T&& val) {
                    this->val = std::move(val);
                }
            };

            std::coroutine_handle<promise_type> coro_handle;
            
            bool await_ready() noexcept {
                return coro_handle.done();
            }
            
            void await_suspend(std::coroutine_handle<> handle) {
                coro_handle.promise().waiting = handle;
            }
            
            T await_resume() {
                T val = std::move(coro_handle.promise().val);
                bool has_error = coro_handle.promise().has_error;
                std::exception_ptr exception = coro_handle.promise().exception;
                coro_handle.destroy();
                if (has_error) {
                    std::rethrow_exception(exception);
                }
                return val;
            }
        };
    }
}

#endif
