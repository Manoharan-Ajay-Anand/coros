#ifndef COROS_IO_READ_AWAITER_H
#define COROS_IO_READ_AWAITER_H

#include <coroutine>

namespace coros::base {
    class IoEventListener;

    struct IoReadAwaiter {
        IoEventListener& listener;
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };
}

#endif
