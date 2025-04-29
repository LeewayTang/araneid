#ifndef ARANEID_BASE_TIME_HPP
#define ARANEID_BASE_TIME_HPP

#include <chrono>
#include <cstdint>
#include <string>
namespace araneid {

/**
 * @brief Class to represent time intervals in various units. To create a
 * TimeDelta object, use the constructor with a time string in the format
 * "1h2m3s4ms5us".
 * @details This class allows for the representation of time intervals
 * in different units such as hours, minutes, seconds, milliseconds,
 * and microseconds. It provides methods for parsing time strings,
 * performing arithmetic operations, and comparing time intervals.
 *
 */
class TimeDelta {
 public:
  TimeDelta(std::string time_str);
  TimeDelta operator+(const TimeDelta& other) const;
  bool operator==(const TimeDelta& other) const;
  bool operator!=(const TimeDelta& other) const;
  bool operator<(const TimeDelta& other) const;
  bool operator>(const TimeDelta& other) const;
  bool isZero() const;
  static TimeDelta zero() { return TimeDelta("0s"); }
  std::string toString() const;
  std::chrono::microseconds toMicroseconds() const;

 private:
  uint32_t microseconds_;
  uint32_t milliseconds_;
  uint32_t seconds_;
  uint32_t minutes_;
  uint32_t hours_;
};

class Clock {
 public:
  static Clock& GetClock() {
    static Clock instance;
    return instance;
  }
  static std::string GetCurrentTimestamp() {
    return GetClock().GetCurrentTime();
  }
  static std::chrono::time_point<std::chrono::system_clock>
  GetCurrentTimePoint() {
    return GetClock().current_time_;
  }
  Clock(const Clock&) = delete;
  Clock& operator=(const Clock&) = delete;
  Clock(Clock&&) = delete;
  Clock& operator=(Clock&&) = delete;

  std::string GetCurrentTime() const;

  int64_t GetCurrentTimeInSeconds() const;
  int64_t GetCurrentTimeInMilliseconds() const;
  int64_t GetCurrentTimeInMicroseconds() const;

 private:
  Clock();
  std::chrono::time_point<std::chrono::system_clock> current_time_;
};
}  // namespace araneid

#endif