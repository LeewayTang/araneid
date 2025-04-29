#ifndef ARANEID_BASE_THREAD_POOL_HPP
#define ARANEID_BASE_THREAD_POOL_HPP

#include "callback.hpp"

namespace araneid {
class ThreadPool {
 public:
  explicit ThreadPool(size_t num_threads);
  ~ThreadPool();
  void Enqueue(std::shared_ptr<CallbackBase> task);

 private:
  std::vector<std::thread> workers_;
  std::queue<std::shared_ptr<CallbackBase>> tasks_;
  std::mutex queue_mutex_;
  std::condition_variable condition_;
  std::atomic<bool> stop_;
};
}  // namespace araneid

#endif