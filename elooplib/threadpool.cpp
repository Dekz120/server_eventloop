#include "threadpool.hpp"

ThreadPool::ThreadPool(size_t num_threads)
{
  threads.reserve(num_threads);
  for (size_t i = 0; i < num_threads; i++)
  {
    threads.emplace_back(&ThreadPool::run, this);
  }
}

void ThreadPool::addTask(std::shared_ptr<ITask> &task)
{
  std::lock_guard<std::mutex> q_lock(q_mtx);
  t_queue.push(task);
  q_cv.notify_one();
}

void ThreadPool::run()
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

      elem->run();
    }
  }
}

ThreadPool::~ThreadPool()
{
  for (uint32_t i = 0; i < threads.size(); ++i)
  {
    q_cv.notify_all();
    threads[i].join();
  }
}
void ThreadPool::stop() { quite = 1; }
