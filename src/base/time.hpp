#ifndef ARANEID_BASE_TIME_HPP
#define ARANEID_BASE_TIME_HPP

#include <chrono>
#include <cstdint>
#include <string>
namespace araneid {

class TimeDelta {
 public:
  // 默认构造（0时长）
  TimeDelta() = default;

  // 从chrono::duration构造
  template <typename Rep, typename Period>
  explicit TimeDelta(const std::chrono::duration<Rep, Period>& duration)
      : duration_(duration) {}

  // 转换为chrono::duration（兼容std库）
  template <typename Duration = std::chrono::nanoseconds>
  Duration ToChrono() const {
    return std::chrono::duration_cast<Duration>(duration_);
  }

  // 常用单位转换
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

  // 运算符重载
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

  // 比较运算符
  bool operator==(const TimeDelta& other) const {
    return duration_ == other.duration_;
  }
  bool operator<(const TimeDelta& other) const {
    return duration_ < other.duration_;
  }
  // 时间格式化
  std::string ToString() const;

 private:
  std::chrono::nanoseconds duration_{0};  // 内部存储为纳秒精度
};

// 便捷构造函数（工厂函数）
TimeDelta Hours(int64_t hours);
TimeDelta Minutes(int64_t minutes);
TimeDelta Seconds(int64_t seconds);
TimeDelta Millis(int64_t ms);
TimeDelta Micros(int64_t us);
TimeDelta Nanos(int64_t ns);

class TimePoint {
 public:
  TimePoint() = default;

  // 从chrono::time_point构造
  template <typename ClockType, typename Duration>
  explicit TimePoint(const std::chrono::time_point<ClockType, Duration>& tp)
      : time_point_(tp) {}

  // 时间算术
  TimePoint operator+(const TimeDelta& delta) const;
  TimePoint operator-(const TimeDelta& delta) const;
  TimeDelta operator-(const TimePoint& other) const;

  // 比较运算符
  bool operator<(const TimePoint& other) const;
  bool operator==(const TimePoint& other) const;
  // ... 其他比较运算符

  // 转换为字符串（UTC时间）
  std::string ToString() const;

  // 转换为chrono::time_point（兼容std库）
  template <typename ClockType = std::chrono::system_clock>
  std::chrono::time_point<ClockType> ToChrono() const {
    return std::chrono::time_point<ClockType>(time_point_);
  }

 private:
  std::chrono::system_clock::time_point time_point_;  // 默认使用系统时钟
  friend class Clock;  // 允许Clock类访问内部time_point_
};

class Clock {
 public:
  static TimePoint Now();  // 默认系统时钟
};
}  // namespace araneid

#endif