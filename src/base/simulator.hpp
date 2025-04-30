#ifndef ARANEID_BASE_SIMULATOR_HPP
#define ARANEID_BASE_SIMULATOR_HPP

#include "scheduler.hpp"

namespace araneid {

class Simulator {
 public:
  // Singleton instance
  static Simulator& Instance() {
    static Simulator instance;
    return instance;
  }

  template <typename T, typename R, typename... Args>
  static void Schedule(TimePoint execution_time, R (T::*func)(Args...),
                       T* instance, Args&&... args);

  template <typename T, typename R, typename... Args>
  static void Schedule(TimePoint execution_time, TimeDelta interval,
                       R (T::*func)(Args...), T* instance, Args&&... args);

  void Start();
  void Destroy();

 private:
  Simulator();

  TimeDelta simulation_duration_time_;
  std::unique_ptr<TimedTaskQueue> scheduler_;
};

}  // namespace araneid

#endif