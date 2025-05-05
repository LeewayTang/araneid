#include "units.hpp"

namespace araneid {

DataRate DataSize::operator/(const TimeDelta& time_delta) const {
  return DataRate(1.0 * bits_ / time_delta.Millis());
}
TimeDelta DataSize::operator/(const DataRate& data_rate) const {
  return TimeDelta::Micros(
      static_cast<int64_t>(bits_ / data_rate.BitsPerSecond() * 1e6));
}

}  // namespace araneid