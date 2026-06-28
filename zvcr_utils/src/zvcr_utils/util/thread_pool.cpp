#include <zvcr_utils/util/thread_pool.hpp>

namespace zvcr {
    auto ThreadPool::init(const size_t threads) -> void {
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

    auto ThreadPool::shutdown() -> void {
        if (stop) return;
        {
            std::unique_lock lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }

    ThreadPool::ThreadPool(): ThreadPool(std::thread::hardware_concurrency()) {}

    ThreadPool::ThreadPool(const size_t threads): ThreadPool(threads, 16384) {}

    ThreadPool::ThreadPool(const size_t threads, const size_t maxQueueSize): slots(maxQueueSize), stop(false) {
        init(threads);
    }

    auto ThreadPool::queueSize() const -> size_t {
        std::unique_lock lock(queueMutex);
        return tasks.size();
    }

    ThreadPool::~ThreadPool() {
        shutdown();
    }
}