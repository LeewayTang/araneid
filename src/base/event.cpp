#include "event.hpp"

#include "log.hpp"

namespace araneid {

template <typename T, typename F, typename... Args>
void Event::Add(T* instance, F func, Args&&... args) {
  LOG_DEBUG << "Event::Add: Adding callback to event with wait time: "
             << wait_time_delta_.toString();
  callbacks_.Enqueue(std::make_unique<Callback<T, F>>(
      func, instance, std::forward<Args>(args)...));
}

void Event::Trigger() {
  LOG_DEBUG << "Event::Trigger: Triggering event with wait time: "
             << wait_time_delta_.toString();
  callbacks_.ExecuteAll();
}

EventManager::EventManager() : stop_(false) {}

template <typename T, typename F, typename... Args>
void EventManager::AddEvent(const TimeDelta& wait_time, T* instance, F func,
                            Args&&... args) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (events_.find(wait_time) == events_.end()) {
    events_[wait_time] = std::make_shared<Event>(wait_time);
  }
  events_[wait_time]->Add(instance, func, std::forward<Args>(args)...);
}

void EventManager::Start() {
  auto queue = std::make_shared<std::priority_queue<std::shared_ptr<Event>>>();
  for (auto& event_pair : events_) {
    event_pair.second->SetStartTime(Clock::GetCurrentTimePoint());
    queue->push(event_pair.second);
  }
  events_.clear();

  worker_ = std::thread([this, queue]() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!stop_) {
      if (queue->empty()) {
        stop_ = true;
        break;
      }
      auto event = std::move(queue->top());
      auto now = Clock::GetCurrentTimePoint();
      if (now >= event->GetExpireTime()) {
        queue->pop();
        lock.unlock();
        event->Trigger();
        lock.lock();
      } else {
        cv_.wait_until(lock, event->GetExpireTime());
      }
    }
  });
}

void EventManager::Stop() {
  stop_.store(true);
  cv_.notify_all();
  if (worker_.joinable()) {
    worker_.join();
  }
}
}  // namespace araneid