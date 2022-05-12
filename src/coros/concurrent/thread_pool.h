#ifndef CONCURRENT_THREAD_POOL_H
#define CONCURRENT_THREAD_POOL_H

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace coros {
    namespace concurrent {
        class ThreadPool {
            private:
                const int max_threads;
                bool shutdown;
                std::vector<std::thread> threads;
                std::queue<std::function<void()>> jobs;
                std::mutex jobs_mutex;
                std::condition_variable jobs_available;
            public:
                ThreadPool();
                ThreadPool(int max_threads);
                ~ThreadPool();
                bool is_shut_down() const;
                void execute_next_job();
                void execute(std::function<void()> job);
        };
        void thread_execute(ThreadPool& thread_pool);
    }
}

#endif
