#include "write_awaiter.h"
#include "listener.h"

#include <coroutine>

bool coros::base::IoWriteAwaiter::await_ready() noexcept {
    return false;
}

void coros::base::IoWriteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    listener.set_write_handle(handle);
}

void coros::base::IoWriteAwaiter::await_resume() {
}
