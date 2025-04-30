#include "scheduler.hpp"

namespace araneid {

template <typename T, typename R, typename... Args>
TimedTask::TimedTask(TimePoint execution_time, R (T::*func)(Args...),
                     T* instance, Args&&... args)
    : execution_time_(execution_time),
      interval_(TimeDelta::Zero()),
      callback_(std::make_shared<Callback<T, R (T::*)(Args...)>>(
          func, instance, std::forward<Args>(args)...)),
      is_periodic_(false) {}

template <typename T, typename R, typename... Args>
TimedTask::TimedTask(TimePoint execution_time, TimeDelta interval,
                     R (T::*func)(Args...), T* instance, Args&&... args)
    : execution_time_(execution_time),
      interval_(interval),
      callback_(std::make_shared<Callback<T, R (T::*)(Args...)>>(
          func, instance, std::forward<Args>(args)...)),
      is_periodic_(true) {}

void TimedTask::Repeat() {
  if (is_periodic_) {
    execution_time_ = execution_time_ + interval_;
  }
}

std::shared_ptr<CallbackBase> TimedTask::SendCallback() {
  return std::move(callback_);
}

TimedTaskQueue::TimedTaskQueue(
    size_t workers = std::thread::hardware_concurrency())
    : stop_(false), thread_pool_(std::make_shared<ThreadPool>(workers)) {}

TimedTaskQueue::~TimedTaskQueue() {
  stop_.store(true);
  cv_.notify_all();
  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }
}

void TimedTaskQueue::Start() {
  if (!stop_.exchange(false)) {
    return;
  }
  worker_thread_ = std::thread([this]() {
    while (!stop_) {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      if (task_queue_.empty()) {
        cv_.wait(lock, [this] { return stop_ || !task_queue_.empty(); });
      }
      if (stop_) {
        return;
      }
      if (task_queue_.empty()) {
        continue;
      }
      TimePoint next_trigger = task_queue_.top().GetExecutionTime();
      if (cv_.wait_until(lock, next_trigger.ToChrono()) ==
          std::cv_status::timeout) {
        ProcessExpiredTasks(lock);
      }
    }
  });
}

void TimedTaskQueue::AddTask(const TimedTask& task) {
  std::unique_lock<std::mutex> lock(queue_mutex_);
  task_queue_.push(task);
  cv_.notify_one();
}

void TimedTaskQueue::ProcessExpiredTasks(std::unique_lock<std::mutex>& lock) {
  TimePoint now = Clock::Now();
  while (!task_queue_.empty() && task_queue_.top().GetExecutionTime() <= now) {
    TimedTask task = task_queue_.top();
    task_queue_.pop();
    lock.unlock();
    thread_pool_->Enqueue(task.SendCallback());
    lock.lock();
    if (task.IsPeriodic()) {
      task.Repeat();
      task_queue_.push(task);
    }
  }
}

}  // namespace araneid