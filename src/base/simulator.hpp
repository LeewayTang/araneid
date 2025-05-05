#ifndef ARANEID_BASE_SIMULATOR_HPP
#define ARANEID_BASE_SIMULATOR_HPP
#include <atomic>
#include <condition_variable>
#include <thread>

#include "callback.hpp"
#include "thread-pool.hpp"
#include "time.hpp"

namespace araneid {
// The core of simulation is timed task scheduling, which is implemented
// using a priority queue. Every task has an execution time and an optional
// interval. The task is executed when the current time reaches the
// execution time. If the task is periodic, it will be rescheduled with
// the same interval.
class TimedTask {
 public:
  template <typename T, typename R, typename... Args>
  TimedTask(TimePoint execution_time, R (T::*func)(Args...), T* instance,
            Args&&... args)
      : execution_time_(execution_time),
        interval_(TimeDelta::Zero()),
        callback_(std::make_shared<Callback<T, R (T::*)(Args...)>>(
            func, instance, std::forward<Args>(args)...)),
        is_periodic_(false) {}

  template <typename T, typename R, typename... Args>
  TimedTask(TimePoint execution_time, TimeDelta interval, R (T::*func)(Args...),
            T* instance, Args&&... args)
      : execution_time_(execution_time),
        interval_(interval),
        callback_(std::make_shared<Callback<T, R (T::*)(Args...)>>(
            func, instance, std::forward<Args>(args)...)),
        is_periodic_(true) {}

  TimePoint GetExecutionTime() const { return execution_time_; }
  void Repeat();
  bool IsPeriodic() const { return is_periodic_; }
  bool operator<(const TimedTask& other) const {
    return execution_time_ < other.execution_time_;
  }
  bool operator>(const TimedTask& other) const {
    return execution_time_ > other.execution_time_;
  }
  std::shared_ptr<CallbackBase> GetCallback();

 private:
  TimePoint execution_time_;
  TimeDelta interval_;
  std::shared_ptr<CallbackBase> callback_;
  bool is_periodic_;
};

// The simulator runs in a separate thread and uses a thread pool to
// execute tasks. The main thread can schedule tasks and wait for
// completion. The simulator can be started and stopped, and it will
// automatically stop when the main thread exits.
// The simulator uses a singleton pattern to ensure that there is only
// one instance of the simulator in the program. The singleton instance
// can be accessed using the Instance() method. The simulator is
// thread-safe and can be used from multiple threads. The simulator
// uses a condition variable to wake up the worker thread when there
// are tasks to be executed. The worker thread will wait for the
// condition variable to be notified when there are tasks to be
// executed. The worker thread will also check if the simulation
// duration has been reached and stop the simulation if it has.
class Simulator {
 public:
  // Singleton instance
  static Simulator& Instance() {
    static Simulator instance;
    return instance;
  }
  ~Simulator();

  template <typename T, typename R, typename... Args>
  void Schedule(TimeDelta execution_wait, R (T::*func)(Args...), T* instance,
                Args&&... args);

  template <typename T, typename R, typename... Args>
  void Schedule(TimeDelta execution_wait, TimeDelta interval,
                R (T::*func)(Args...), T* instance, Args&&... args);

  void Start(TimeDelta simulation_duration);
  void Stop();

 private:
  Simulator();
  void ProcessExpiredTasks(std::unique_lock<std::mutex>& lock);
  std::priority_queue<TimedTask> task_queue_;
  std::mutex queue_mutex_;
  std::condition_variable cv_;
  std::atomic<bool> stop_;
  std::thread worker_thread_;
  std::shared_ptr<ThreadPool> thread_pool_;
  TimePoint simulation_start_time_;
  TimeDelta simulation_least_duration_;
};

}  // namespace araneid

#endif  // ARANEID_BASE_SIMULATOR_HPP