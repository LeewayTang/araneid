#ifndef ARANEID_BASE_SCHEDULER_HPP
#define ARANEID_BASE_SCHEDULER_HPP

#include <thread>

#include "callback.hpp"
#include "thread-pool.hpp"
#include "time.hpp"
namespace araneid {
class TimedTask {
 public:
  template <typename T, typename R, typename... Args>
  TimedTask(TimePoint execution_time, R (T::*func)(Args...), T* instance,
            Args&&... args);

  template <typename T, typename R, typename... Args>
  TimedTask(TimePoint execution_time, TimeDelta interval, R (T::*func)(Args...),
            T* instance, Args&&... args);

  TimePoint GetExecutionTime() const { return execution_time_; }
  void Repeat();
  bool IsPeriodic() const { return is_periodic_; }
  bool operator<(const TimedTask& other) const {
    return execution_time_ < other.execution_time_;
  }
  bool operator>(const TimedTask& other) const {
    return execution_time_ > other.execution_time_;
  }
  std::shared_ptr<CallbackBase> SendCallback();

 private:
  TimePoint execution_time_;
  TimeDelta interval_;
  std::shared_ptr<CallbackBase> callback_;
  bool is_periodic_;
};

class TimedTaskQueue {
 public:
  TimedTaskQueue(size_t workers);
  ~TimedTaskQueue();
  void Start();
  void AddTask(const TimedTask& task);

 private:
  void ProcessExpiredTasks(std::unique_lock<std::mutex>& lock);
  std::priority_queue<TimedTask> task_queue_;
  std::mutex queue_mutex_;
  std::condition_variable cv_;
  std::atomic<bool> stop_;
  std::thread worker_thread_;
  std::shared_ptr<ThreadPool> thread_pool_;
};

}  // namespace araneid

#endif