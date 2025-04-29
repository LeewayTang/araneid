#ifndef ARANEID_BASE_EVENT_HPP
#define ARANEID_BASE_EVENT_HPP

#include <condition_variable>
#include <map>
#include <thread>
#include <vector>

#include "callback.hpp"
#include "time.hpp"

namespace araneid {

// An event is a collection of callbacks that are triggered after a certain time.
// Users should have the freedom to add callbacks to an event so that they can control
// the simulation flow.
class Event {
 public:
  explicit Event(TimeDelta expire_time) : wait_time_delta_(expire_time) {}

  bool operator<(const Event& other) const {
    return wait_time_delta_ < other.wait_time_delta_;
  }
  bool operator>(const Event& other) const {
    return wait_time_delta_ > other.wait_time_delta_;
  }
  bool operator==(const Event& other) const {
    return wait_time_delta_ == other.wait_time_delta_;
  }
  bool operator!=(const Event& other) const {
    return wait_time_delta_ != other.wait_time_delta_;
  }
  // Add a callback to the event
  template <typename T, typename F, typename... Args>
  void Add(T* instance, F func, Args&&... args);

  // Execute all callbacks
  void Trigger();

  // Set the expiration time of the event with the current time
  void SetStartTime(
      const std::chrono::time_point<std::chrono::system_clock>& start_time) {
    expire_time_ = start_time + wait_time_delta_.toMicroseconds();
  }

  // Get the expiration time of the event
  std::chrono::time_point<std::chrono::system_clock> GetExpireTime() const {
    return expire_time_;
  }

 private:
  TimeDelta wait_time_delta_;
  std::chrono::time_point<std::chrono::system_clock> expire_time_;
  CallbackQueue callbacks_;
};

// Even though users can freely add callbacks to an event, a manager is needed to
// manage the events and trigger them at the right time. The manager will run in a
// separate thread and will be responsible for checking the events and triggering
// them when the time is up.
class EventManager {
 public:
  EventManager();
  ~EventManager();
  template <typename T, typename F, typename... Args>
  void AddEvent(const TimeDelta& wait_time, T* instance, F func,
                Args&&... args);
  void Start();
  void Stop();

 private:
  std::atomic<bool> stop_;
  std::mutex mutex_;
  std::thread worker_;
  std::condition_variable cv_;
  std::map<TimeDelta, std::shared_ptr<Event>> events_;
};

}  // namespace araneid

#endif