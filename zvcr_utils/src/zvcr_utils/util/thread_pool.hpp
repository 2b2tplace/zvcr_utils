#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <semaphore>
#include <stdexcept>
#include <thread>
#include <vector>

namespace zvcr {

    class ThreadPool {
    public:
        explicit ThreadPool(size_t threads, size_t maxQueueSize);

        explicit ThreadPool(size_t threads);

        ThreadPool();

        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
            using return_type = std::invoke_result_t<F, Args...>;

            slots.acquire();

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            auto res = task->get_future();
            {
                std::unique_lock lock(queueMutex);
                if (stop) {
                    slots.release();
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                }
                tasks.emplace([this, task] {
                    (*task)();
                    slots.release(); // open a slot for the next producer
                });
            }
            condition.notify_one();
            return res;
        }

        [[nodiscard]]
        auto queueSize() const -> size_t;

        ~ThreadPool();

        auto shutdown() -> void;
    private:
        auto init(size_t threads) -> void;

        std::counting_semaphore<> slots;
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        mutable std::mutex queueMutex;
        std::condition_variable condition;
        bool stop;
    };

}