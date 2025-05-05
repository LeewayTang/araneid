#ifndef ARANEID_BASE_TIME_HPP
#define ARANEID_BASE_TIME_HPP

#include <chrono>
#include <cstdint>
#include <string>
namespace araneid {

class TimeDelta {
 public:
  TimeDelta() = default;

  template <typename Rep, typename Period>
  explicit TimeDelta(const std::chrono::duration<Rep, Period>& duration)
      : duration_(duration) {}

  template <typename Duration = std::chrono::nanoseconds>
  Duration ToChrono() const {
    return std::chrono::duration_cast<Duration>(duration_);
  }

  int64_t Hours() const {
    return std::chrono::duration_cast<std::chrono::hours>(duration_).count();
  }
  int64_t Minutes() const {
    return std::chrono::duration_cast<std::chrono::minutes>(duration_).count();
  }
  int64_t Seconds() const {
    return std::chrono::duration_cast<std::chrono::seconds>(duration_).count();
  }
  int64_t Millis() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration_)
        .count();
  }
  int64_t Micros() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(duration_)
        .count();
  }
  int64_t Nanos() const {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration_)
        .count();
  }

  TimeDelta operator+(const TimeDelta& other) const {
    return TimeDelta(duration_ + other.duration_);
  }
  TimeDelta operator-(const TimeDelta& other) const {
    return TimeDelta(duration_ - other.duration_);
  }
  TimeDelta& operator+=(const TimeDelta& other) {
    duration_ += other.duration_;
    return *this;
  }
  TimeDelta& operator-=(const TimeDelta& other) {
    duration_ -= other.duration_;
    return *this;
  }

  bool operator==(const TimeDelta& other) const {
    return duration_ == other.duration_;
  }
  bool operator<(const TimeDelta& other) const {
    return duration_ < other.duration_;
  }
  bool operator>(const TimeDelta& other) const {
    return duration_ > other.duration_;
  }
  bool operator<=(const TimeDelta& other) const {
    return duration_ <= other.duration_;
  }
  bool operator>=(const TimeDelta& other) const {
    return duration_ >= other.duration_;
  }
  std::string ToString() const;

  static TimeDelta Hours(int64_t hours) {
    return TimeDelta(std::chrono::hours(hours));
  }
  static TimeDelta Minutes(int64_t minutes) {
    return TimeDelta(std::chrono::minutes(minutes));
  }
  static TimeDelta Seconds(int64_t seconds) {
    return TimeDelta(std::chrono::seconds(seconds));
  }
  static TimeDelta Millis(int64_t ms) {
    return TimeDelta(std::chrono::milliseconds(ms));
  }
  static TimeDelta Micros(int64_t us) {
    return TimeDelta(std::chrono::microseconds(us));
  }
  static TimeDelta Nanos(int64_t ns) {
    return TimeDelta(std::chrono::nanoseconds(ns));
  }
  static TimeDelta Zero() { return TimeDelta(std::chrono::nanoseconds(0)); }

 private:
  std::chrono::nanoseconds duration_{0};
};

class TimePoint {
 public:
  TimePoint() = default;

  template <typename ClockType, typename Duration>
  explicit TimePoint(const std::chrono::time_point<ClockType, Duration>& tp)
      : time_point_(tp) {}

  TimePoint operator+(const TimeDelta& delta) const {
    return TimePoint(time_point_ + delta.ToChrono());
  }
  TimePoint operator-(const TimeDelta& delta) const {
    return TimePoint(time_point_ - delta.ToChrono());
  }
  TimeDelta operator-(const TimePoint& other) const {
    return TimeDelta(time_point_ - other.time_point_);
  }

  bool operator<(const TimePoint& other) const {
    return time_point_ < other.time_point_;
  }
  bool operator>(const TimePoint& other) const {
    return time_point_ > other.time_point_;
  }
  bool operator<=(const TimePoint& other) const { return !(*this > other); }
  bool operator>=(const TimePoint& other) const { return !(*this < other); }
  bool operator==(const TimePoint& other) const {
    return time_point_ == other.time_point_;
  }

  std::string ToString() const;

  template <typename ClockType = std::chrono::system_clock>
  std::chrono::time_point<ClockType> ToChrono() const {
    return std::chrono::time_point<ClockType>(time_point_);
  }

 private:
  std::chrono::system_clock::time_point time_point_;
  friend class Clock;
};

class Clock {
 public:
  static TimePoint Now();
};
}  // namespace araneid

#endif  // ARANEID_BASE_TIME_HPP