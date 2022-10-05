#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <condition_variable>
#include <queue>

class ThreadPool
{
public:
    ThreadPool(size_t num_threads)
    {
        threads.reserve(num_threads);
        for (size_t i = 0; i < num_threads; i++)
        {
            threads.emplace_back(&ThreadPool::run, this);
        }
    }
    ThreadPool(ThreadPool&) = delete; 
    ThreadPool& operator=(ThreadPool&) = delete; 
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete; 
                                   //threads(std::move(rhs.threads)){};

    template <typename Func, typename... Args>
    void add_task(const Func &task_func, Args &&...args)
    {
        std::cout << "Add obj to threadpool\n";
        std::lock_guard<std::mutex> q_lock(q_mtx);
        t_queue.emplace(std::async(std::launch::deferred, task_func, args...));
        q_cv.notify_one();
    }

    static std::shared_ptr<ThreadPool> getInstance()
    {
        if (!p_instance)
        {
            p_instance = std::shared_ptr<ThreadPool>(new ThreadPool(3));
        }
        return p_instance;
    }

    ~ThreadPool()
    {
        for (uint32_t i = 0; i < threads.size(); ++i)
        {
            q_cv.notify_all();
            threads[i].join();
        }
    }

    void stop()
    {
        quite =1;
    }
private:
    static inline std::shared_ptr<ThreadPool> p_instance = nullptr;

    std::queue<std::future<void>> t_queue;
    std::mutex q_mtx;
    std::condition_variable q_cv;

    std::vector<std::thread> threads;
    std::atomic<bool> quite{0};

    void run()
    {
        while (!quite)
        {
            std::unique_lock<std::mutex> lock(q_mtx);
            q_cv.wait(lock, [this]()
                      { return !t_queue.empty() || quite; });

            if (!t_queue.empty())
            {
                auto elem = std::move(t_queue.front());
                t_queue.pop();
                lock.unlock();

                elem.get();
            }
        }
    }
};