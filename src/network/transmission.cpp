#include "transmission.hpp"

#include <random>

#include "base/log.hpp"

namespace araneid {

RandomPacketLoss::RandomPacketLoss(double drop_rate) : drop_rate_(drop_rate) {
  if (drop_rate < 0.0 || drop_rate > 1.0) {
    ALOG_ERROR << "Invalid drop rate: " << drop_rate;
  }
}

bool RandomPacketLoss::ShouldDropPacket(const Packet& packet) {
  // Use a random number generator to determine whether to drop the packet
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, 1.0);
  double random_value = dis(gen);
  return random_value < drop_rate_;
}





}  // namespace araneid