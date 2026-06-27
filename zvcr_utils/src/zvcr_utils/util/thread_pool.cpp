#include <zvcr_utils/util/thread_pool.hpp>

namespace zvcr {
    void ThreadPool::init(const size_t threads) {
        for (size_t i = 0; i < threads; i++) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(this->queueMutex);
                        this->condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); });

                        if (this->stop && this->tasks.empty()) return;

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    void ThreadPool::shutdown() {
        if (stop) return;
        {
            std::unique_lock lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }

    ThreadPool::ThreadPool(): stop(false) {
        init(std::thread::hardware_concurrency());
    }

    ThreadPool::ThreadPool(const size_t threads): stop(false) {
        init(threads);
    }

    size_t ThreadPool::queueSize() const {
        return tasks.size();
    }

    ThreadPool::~ThreadPool() {
        shutdown();
    }
}