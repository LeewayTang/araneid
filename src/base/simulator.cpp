#include "simulator.hpp"

namespace araneid {

template <typename T, typename R, typename... Args>
void Simulator::Schedule(TimePoint execution_time, R (T::*func)(Args...),
                         T* instance, Args&&... args) {
  TimedTask task(execution_time, func, instance, std::forward<Args>(args)...);
  scheduler_->AddTask(task);
}

template <typename T, typename R, typename... Args>
void Simulator::Schedule(TimePoint execution_time, TimeDelta interval,
                         R (T::*func)(Args...), T* instance, Args&&... args) {
  TimedTask task(execution_time, interval, func, instance,
                 std::forward<Args>(args)...);
  scheduler_->AddTask(task);
}

}  // namespace araneid