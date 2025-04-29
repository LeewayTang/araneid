#include "time.hpp"

#include <iostream>

namespace araneid {

TimeDelta::TimeDelta(std::string time_str) {
  /**
   * @brief Parse a time string in different units (e.g., "1h", "1h2m",
   * "1h2m3s", "1h2m3s4ms", "4ms5us")
   * @param time_str The time string to parse
   */
  // Initialize all time components to zero
  microseconds_ = 0;
  milliseconds_ = 0;
  seconds_ = 0;
  minutes_ = 0;
  hours_ = 0;
  // Parse the time string

  size_t pos = 0;
  while (pos < time_str.size()) {
    size_t start = pos;
    while (pos < time_str.size() && isdigit(time_str[pos])) {
      ++pos;
    }
    if (start == pos) {
      throw std::invalid_argument("Invalid time format");
      break;  // No digits found
    }
    int value = std::stoi(time_str.substr(start, pos - start));
    if (pos < time_str.size()) {
      char unit = time_str[pos++];
      switch (unit) {
        case 'h':
          hours_ += value;
          break;
        case 'm':
          minutes_ += value;
          break;
        case 's':
          seconds_ += value;
          break;
        case 'ms':
          milliseconds_ += value;
          break;
        case 'us':
          microseconds_ += value;
          break;
        default:
          throw std::invalid_argument("Invalid time unit");
      }
    }
  }
}

TimeDelta TimeDelta::operator+(const TimeDelta& other) const {
  TimeDelta result = zero();
  result.microseconds_ = microseconds_ + other.microseconds_;
  result.milliseconds_ = milliseconds_ + other.milliseconds_;
  result.seconds_ = seconds_ + other.seconds_;
  result.minutes_ = minutes_ + other.minutes_;
  result.hours_ = hours_ + other.hours_;
  return result;
}

bool TimeDelta::operator==(const TimeDelta& other) const {
  return microseconds_ == other.microseconds_ &&
         milliseconds_ == other.milliseconds_ && seconds_ == other.seconds_ &&
         minutes_ == other.minutes_ && hours_ == other.hours_;
}

bool TimeDelta::operator!=(const TimeDelta& other) const {
  return !(*this == other);
}

bool TimeDelta::operator<(const TimeDelta& other) const {
  if (hours_ != other.hours_) {
    return hours_ < other.hours_;
  }
  if (minutes_ != other.minutes_) {
    return minutes_ < other.minutes_;
  }
  if (seconds_ != other.seconds_) {
    return seconds_ < other.seconds_;
  }
  if (milliseconds_ != other.milliseconds_) {
    return milliseconds_ < other.milliseconds_;
  }
  return microseconds_ < other.microseconds_;
}

bool TimeDelta::operator>(const TimeDelta& other) const {
  return !(*this < other) && !(*this == other);
}

bool TimeDelta::isZero() const {
  return microseconds_ == 0 && milliseconds_ == 0 && seconds_ == 0 &&
         minutes_ == 0 && hours_ == 0;
}

std::string TimeDelta::toString() const {
  std::string result;
  if (hours_ > 0) {
    result += std::to_string(hours_) + "h";
  }
  if (minutes_ > 0) {
    result += std::to_string(minutes_) + "m";
  }
  if (seconds_ > 0) {
    result += std::to_string(seconds_) + "s";
  }
  if (milliseconds_ > 0) {
    result += std::to_string(milliseconds_) + "ms";
  }
  if (microseconds_ > 0) {
    result += std::to_string(microseconds_) + "us";
  }
  return result.empty() ? "0s" : result;
}

std::chrono::microseconds TimeDelta::toMicroseconds() const {
  return std::chrono::microseconds(microseconds_ + milliseconds_ * 1000 +
                                   seconds_ * 1000000 + minutes_ * 60000000 +
                                   hours_ * 3600000000);
}

Clock::Clock() : current_time_(std::chrono::system_clock::now()) {}

int64_t Clock::GetCurrentTimeInSeconds() const {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now() - current_time_)
      .count();
}

int64_t Clock::GetCurrentTimeInMilliseconds() const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now() - current_time_)
      .count();
}

int64_t Clock::GetCurrentTimeInMicroseconds() const {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::system_clock::now() - current_time_)
      .count();
}

std::string Clock::GetCurrentTime() const {
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  std::tm tm_local{};
  localtime_r(&time_t_now, &tm_local);
  char buffer[100];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_local);
  return std::string(buffer);
}

}  // namespace araneid
