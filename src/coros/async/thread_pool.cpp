#include "thread_pool.h"

#include <iostream>
#include <thread>
#include <functional>
#include <stdexcept>
#include <mutex>

coros::async::ThreadPool::ThreadPool() : max_threads(std::thread::hardware_concurrency()) {
    is_shutdown = false;
}

coros::async::ThreadPool::ThreadPool(int max_threads) : max_threads(max_threads) {
    is_shutdown = false;    
}

void coros::async::ThreadPool::run_jobs() {
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> jobs_lock(jobs_mutex);
            jobs_condition.wait(jobs_lock, [&] { return !jobs.empty() || is_shutdown;});
            if (jobs.empty() && is_shutdown) {
                return;
            }
            job = jobs.front();
            jobs.pop();
        }
        job();
    }
}

void coros::async::ThreadPool::run(std::function<void()> job) {
    if (is_shutdown) {
        throw std::runtime_error("ThreadPool run() error: ThreadPool already shutdown");
    }
    {
        std::lock_guard<std::mutex> guard(jobs_mutex);
        jobs.push(job);
        if (threads.size() < max_threads) {
            threads.push_back(std::thread(&ThreadPool::run_jobs, this));
        }
    }
    jobs_condition.notify_one();
}

void coros::async::ThreadPool::shutdown() {
    if (is_shutdown) {
        throw std::runtime_error("ThreadPool shutdown() error: ThreadPool already shutdown");
    }
    is_shutdown = true;
    jobs_condition.notify_all();
    for (auto it = threads.begin(); it != threads.end(); it++) {
        it->join();
    }
}
