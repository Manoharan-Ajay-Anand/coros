#include "read_awaiter.h"
#include "listener.h"

#include <coroutine>

bool coros::base::IoReadAwaiter::await_ready() noexcept {
    return false;
}

void coros::base::IoReadAwaiter::await_suspend(std::coroutine_handle<> handle) {
    listener.set_read_handle(handle);
}

void coros::base::IoReadAwaiter::await_resume() {
}
