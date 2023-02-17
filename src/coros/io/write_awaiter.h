#ifndef COROS_IO_WRITE_AWAITER_H
#define COROS_IO_WRITE_AWAITER_H

#include <coroutine>

namespace coros::base {
    class IoEventListener;

    struct IoWriteAwaiter {
        IoEventListener& listener;
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume();
    };
}

#endif
