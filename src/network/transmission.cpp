#include "transmission.hpp"

#include <random>

#include "base/log.hpp"
#include "base/simulator.hpp"

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

void RandomPacketLoss::SetLossRate(double drop_rate) {
  if (drop_rate < 0.0 || drop_rate > 1.0) {
    ALOG_ERROR << "Invalid drop rate: " << drop_rate;
    return;
  }
  drop_rate_ = drop_rate;
}

CommonTransmission::CommonTransmission(std::unique_ptr<PacketLoss> packet_loss,
                                       TimeDelta delay, DataRate bandwidth,
                                       DataSize buffer_size)
    : packet_loss_(std::move(packet_loss)),
      delay_(delay),
      bottleneck_bandwidth_(bandwidth),
      bottleneck_buffer_size_(buffer_size),
      cached_buffer_size_(0) {}

void CommonTransmission::SendToNetwork(std::shared_ptr<Packet> packet) {
  if (!IsConnected()) {
    ALOG_INFO << "Not connected, dropping packet";
    return;
  }
  {
    std::lock_guard<std::mutex> lock(loss_mutex_);
    if (packet_loss_->ShouldDropPacket(*packet)) {
      ALOG_INFO << "Packet dropped by " << packet_loss_->GetName();
      return;
    }
  }
  {
    std::lock_guard<std::mutex> lock(delay_mutex_);
    Simulator::Instance().Schedule(delay_, &CommonTransmission::InFlight, this,
                                   std::move(packet));
  }
}

void CommonTransmission::InFlight(std::shared_ptr<Packet> packet) {
  std::lock_guard<std::mutex> lock(bottleneck_buffer_size_mutex_);
  if (cached_buffer_size_ + packet->GetSize() >= bottleneck_buffer_size_) {
    ALOG_INFO << "Buffer overflow, dropping packet";
    return;
  }
  cached_buffer_size_ += packet->GetSize();
  {
    std::lock_guard<std::mutex> lock(bottleneck_bandwidth_mutex_);
    TimeDelta bottleneck_cached_delay =
        packet->GetSize() / bottleneck_bandwidth_;
    Simulator::Instance().Schedule(bottleneck_cached_delay,
                                   &CommonTransmission::ReceiveFromNetwork,
                                   this, std::move(packet));
  }
}

void CommonTransmission::ReceiveFromNetwork(std::shared_ptr<Packet> packet) {
  std::lock_guard<std::mutex> lock(bottleneck_buffer_size_mutex_);
  if (cached_buffer_size_ < packet->GetSize()) {
    ALOG_ERROR
        << "Something went wrong, cached buffer size is less than packet size";
    return;
  }
  // Simulate the packet being received by the receiver device
  // and update the cached buffer size.
  cached_buffer_size_ -= packet->GetSize();
  if (receiver_) {
    receiver_->Receive(packet);
  } else {
    ALOG_ERROR << "Receiver is not set, cannot receive packet";
  }
}

void CommonTransmission::SetDelay(TimeDelta delay) {
  std::lock_guard<std::mutex> lock(delay_mutex_);
  delay_ = delay;
}

void CommonTransmission::SetPacketLoss(
    std::unique_ptr<PacketLoss> packet_loss) {
  std::lock_guard<std::mutex> lock(loss_mutex_);
  packet_loss_ = std::move(packet_loss);
}

void CommonTransmission::SetBottleneckBandwidth(DataRate bandwidth) {
  std::lock_guard<std::mutex> lock(bottleneck_bandwidth_mutex_);
  bottleneck_bandwidth_ = bandwidth;
}

void CommonTransmission::SetBottleneckBufferSize(DataSize buffer_size) {
  std::lock_guard<std::mutex> lock(bottleneck_buffer_size_mutex_);
  bottleneck_buffer_size_ = buffer_size;
}

}  // namespace araneid