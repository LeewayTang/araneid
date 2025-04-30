#ifndef ARANEID_BASE_UNITS_HPP
#define ARANEID_BASE_UNITS_HPP

#include <cstdlib>

#include "time.hpp"

namespace araneid {
class DataSize {
 public:
  explicit DataSize(uint64_t bits) : bits_(bits) {}

  uint64_t Bits() const { return bits_; }
  uint64_t Bytes() const { return bits_ / 8; }
  uint64_t KiloBytes() const { return bits_ / 8192; }
  uint64_t MegaBytes() const { return bits_ / 8388608; }
  uint64_t GigaBytes() const { return bits_ / 8589934592; }

  static DataSize Zero() { return DataSize(0); }
  static DataSize Bits(uint64_t bits) { return DataSize(bits); }
  static DataSize Bytes(uint64_t bytes) { return DataSize(bytes * 8); }
  static DataSize KiloBytes(uint64_t kilobytes) {
    return DataSize(kilobytes * 8192);
  }
  static DataSize MegaBytes(uint64_t megabytes) {
    return DataSize(megabytes * 8388608);
  }
  static DataSize GigaBytes(uint64_t gigabytes) {
    return DataSize(gigabytes * 8589934592);
  }

  DataSize operator+(const DataSize& other) const {
    return DataSize(bits_ + other.bits_);
  }
  DataSize operator-(const DataSize& other) const {
    return DataSize(bits_ - other.bits_);
  }
  DataSize operator*(double factor) const {
    return DataSize(static_cast<uint64_t>(bits_ * factor));
  }
  DataRate operator/(const TimeDelta& time_delta) const {
    return DataRate(1.0 * bits_ / time_delta.Millis());
  }

 private:
  uint64_t bits_;
};

class DataRate {
 public:
  explicit DataRate(double bits_per_second)
      : bits_per_second_(bits_per_second) {}

  double BitsPerSecond() const { return bits_per_second_; }
  double BytesPerSecond() const { return bits_per_second_ / 8; }
  double KiloBytesPerSecond() const { return bits_per_second_ / 8192; }
  double MegaBytesPerSecond() const { return bits_per_second_ / 8388608; }
  double GigaBytesPerSecond() const { return bits_per_second_ / 8589934592; }

  static DataRate Zero() { return DataRate(0); }
  static DataRate BitsPerSecond(double bps) { return DataRate(bps); }
  static DataRate BytesPerSecond(double bps) { return DataRate(bps * 8); }
  static DataRate KiloBytesPerSecond(double kbps) {
    return DataRate(kbps * 8192);
  }
  static DataRate MegaBytesPerSecond(double mbps) {
    return DataRate(mbps * 8388608);
  }
  static DataRate GigaBytesPerSecond(double gbps) {
    return DataRate(gbps * 8589934592);
  }
  DataRate operator+(const DataRate& other) const {
    return DataRate(bits_per_second_ + other.bits_per_second_);
  }
  DataRate operator-(const DataRate& other) const {
    return DataRate(bits_per_second_ - other.bits_per_second_);
  }
  DataRate operator*(double factor) const {
    return DataRate(bits_per_second_ * factor);
  }
  DataRate operator/(double factor) const {
    return DataRate(bits_per_second_ / factor);
  }
  DataSize operator*(const TimeDelta& time_delta) const {
    return DataSize(
        static_cast<uint64_t>(bits_per_second_ * time_delta.Millis()));
  }

 private:
  double bits_per_second_;
};

}  // namespace araneid

#endif