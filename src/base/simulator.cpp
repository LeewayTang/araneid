#include "simulator.hpp"

#include <condition_variable>
#include <mutex>

#include "log.hpp"

namespace araneid {
constexpr uint32_t kMaxThreads = 4;

void TimedTask::Repeat() {
  if (is_periodic_) {
    execution_time_ = execution_time_ + interval_;
  }
}

std::shared_ptr<CallbackBase> TimedTask::GetCallback() {
  return std::move(callback_);
}

Simulator::Simulator()
    : stop_(false),
      thread_pool_(std::make_shared<ThreadPool>(
          std::min(kMaxThreads, std::thread::hardware_concurrency()))) {}

Simulator::~Simulator() {
  if (!stop_.load()) {
    Stop();
  }
}

void Simulator::ProcessExpiredTasks(std::unique_lock<std::mutex>& lock) {
  TimePoint now = Clock::Now();
  while (!task_queue_.empty() && task_queue_.top().GetExecutionTime() <= now) {
    TimedTask task = task_queue_.top();
    task_queue_.pop();
    lock.unlock();
    thread_pool_->Enqueue(task.GetCallback());
    lock.lock();
    if (task.IsPeriodic()) {
      task.Repeat();
      task_queue_.push(task);
    }
  }
}

template <typename T, typename R, typename... Args>
void Simulator::Schedule(TimeDelta execution_wait, R (T::*func)(Args...),
                         T* instance, Args&&... args) {
  std::unique_lock<std::mutex> lock(queue_mutex_);
  TimePoint execution_time = simulation_start_time_ + execution_wait;
  TimedTask task(execution_time, func, instance, std::forward<Args>(args)...);
  task_queue_.push(task);
  cv_.notify_one();
}

template <typename T, typename R, typename... Args>
void Simulator::Schedule(TimeDelta execution_wait, TimeDelta interval,
                         R (T::*func)(Args...), T* instance, Args&&... args) {
  std::unique_lock<std::mutex> lock(queue_mutex_);
  TimePoint execution_time = simulation_start_time_ + execution_wait;
  TimedTask task(execution_time, interval, func, instance,
                 std::forward<Args>(args)...);
  task_queue_.push(task);
  cv_.notify_one();
}

void Simulator::Start(TimeDelta simulation_duration) {
  if (simulation_duration <= simulation_least_duration_) {
    ALOG_WARNING << "Simulation duration is less than the least duration, "
                    "there may be some tasks not executed.";
  }
  simulation_start_time_ = Clock::Now();
  Simulator::Instance().Schedule(simulation_duration, &Simulator::Stop, this);
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

void Simulator::Stop() {
  if (stop_.exchange(true)) {
    return;
  }
  cv_.notify_all();
  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }
}

}  // namespace araneid