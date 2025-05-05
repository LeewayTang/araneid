#include "thread-pool.hpp"

#include <mutex>
#include <thread>
namespace araneid {

ThreadPool::ThreadPool(size_t num_threads) : stop_(false) {
  for (size_t i = 0; i < num_threads; ++i) {
    workers_.emplace_back([this] {
      while (true) {
        std::shared_ptr<CallbackBase> task;

        {
          std::unique_lock<std::mutex> lock(queue_mutex_);
          condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
          if (stop_ && tasks_.empty()) {
            return;
          }
          task = std::move(tasks_.front());
          tasks_.pop();
        }

        task->Execute();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  stop_.store(true);
  condition_.notify_all();
  for (auto& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void ThreadPool::Enqueue(std::shared_ptr<CallbackBase> task) {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    tasks_.emplace(std::move(task));
  }
  condition_.notify_one();
}

}  // namespace araneid