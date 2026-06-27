#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace zvcr {

    class ThreadPool {
    public:
        explicit ThreadPool(size_t threads);

        ThreadPool();

        template<class F, class... Args>
        std::future<std::result_of_t<F(Args...)>> enqueue(F &&f, Args &&...args) {
            using return_type = std::result_of_t<F(Args...)>;

            auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<return_type> res = task->get_future();
            {
                std::unique_lock lock(queueMutex);
                if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");

                tasks.emplace([task]{ (*task)(); });
            }
            condition.notify_one();
            return res;
        }

        [[nodiscard]]
        size_t queueSize() const;

        ~ThreadPool();

        void shutdown();
    private:
        void init(size_t threads);

        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queueMutex;
        std::condition_variable condition;
        bool stop;
    };

}