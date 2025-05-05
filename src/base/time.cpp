#include "time.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>
namespace araneid {

TimeDelta Hours(int64_t hours) { return TimeDelta(std::chrono::hours(hours)); }

TimeDelta Minutes(int64_t minutes) {
  return TimeDelta(std::chrono::minutes(minutes));
}

TimeDelta Seconds(int64_t seconds) {
  return TimeDelta(std::chrono::seconds(seconds));
}

TimeDelta Millis(int64_t ms) {
  return TimeDelta(std::chrono::milliseconds(ms));
}

TimeDelta Micros(int64_t us) {
  return TimeDelta(std::chrono::microseconds(us));
}

TimeDelta Nanos(int64_t ns) { return TimeDelta(std::chrono::nanoseconds(ns)); }

std::string TimeDelta::ToString() const {
  auto ns = Nanos();
  const int64_t us = ns / 1000;
  const int64_t ms = us / 1000;
  const int64_t s = ms / 1000;
  std::stringstream ss;
  if (s > 0) {
    ss << s << "s";
  } else if (ms > 0) {
    ss << ms << "ms";
  } else if (us > 0) {
    ss << us << "us";
  } else {
    ss << ns << "ns";
  }
  return ss.str();
}

std::string TimePoint::ToString() const {
  auto in_time_t = std::chrono::system_clock::to_time_t(time_point_);
  std::tm tm;
  gmtime_r(&in_time_t, &tm);  // 线程安全版本（Linux）
  std::stringstream ss;
  ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                time_point_.time_since_epoch())
                .count() %
            1000;
  ss << "." << std::setw(3) << std::setfill('0') << ms;
  return ss.str();
}

TimePoint Clock::Now() { return TimePoint(std::chrono::system_clock::now()); }

}  // namespace araneid
