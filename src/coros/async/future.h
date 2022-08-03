#ifndef COROS_ASYNC_FUTURE_H
#define COROS_ASYNC_FUTURE_H

#include <coroutine>
#include <exception>
#include <optional>

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
            
            std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) noexcept {
                if (waiting) {
                    return waiting;
                }
                return std::noop_coroutine();
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
            
            ~AwaitableFuture() {
                coro_handle.destroy();
            }

            bool await_ready() noexcept {
                return coro_handle.done();
            }
            
            void await_suspend(std::coroutine_handle<> handle) {
                coro_handle.promise().waiting = handle;
            }
            
            void await_resume() {
                promise_type& promise = coro_handle.promise();
                if (promise.has_error) {
                    std::rethrow_exception(promise.exception);
                }
            }
        };

        template<typename T>
        struct AwaitableValue {
            struct promise_type {
                std::optional<T> val;
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
                
                void return_value(const T& t) {
                    val = t;
                }

                void return_value(T&& t) {
                    val = std::move(t);
                }
            };

            std::coroutine_handle<promise_type> coro_handle;

            ~AwaitableValue() {
                coro_handle.destroy();
            }
            
            bool await_ready() noexcept {
                return coro_handle.done();
            }
            
            void await_suspend(std::coroutine_handle<> handle) {
                coro_handle.promise().waiting = handle;
            }
            
            T&& await_resume() {
                promise_type& promise = coro_handle.promise();
                if (promise.has_error) {
                    std::rethrow_exception(promise.exception);
                }
                return std::move(promise.val.value());
            }
        };
    }
}

#endif
