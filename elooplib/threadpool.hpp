#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <future>
#include <condition_variable>
#include <queue>

class ITask
{
public:
    virtual void run()=0;
    ITask() = default;

    virtual ~ITask() = default;
};

class ThreadPool
{
public:
    ThreadPool(size_t);
    ThreadPool(ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    void addTask(std::shared_ptr<ITask> &task);

    void stop();
    ~ThreadPool();

private:
    std::queue<std::shared_ptr<ITask>> t_queue;
    std::mutex q_mtx;
    std::condition_variable q_cv;

    std::vector<std::thread> threads;
    std::atomic<bool> quite{0};
    void run();
};