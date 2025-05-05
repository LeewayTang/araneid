#ifndef ARANEID_NETWORK_TRANSMISSION_HPP
#define ARANEID_NETWORK_TRANSMISSION_HPP

#include <atomic>
#include <queue>

#include "base/units.hpp"
#include "device.hpp"
#include "packet.hpp"

namespace araneid {

class PacketLoss {
 public:
  // This class is responsible for determining whether a packet should be
  // dropped
  virtual bool ShouldDropPacket(const Packet& packet) = 0;
  static std::string GetName() { return "PacketLoss"; }
};

class RandomPacketLoss : public PacketLoss {
 public:
  RandomPacketLoss(double drop_rate);
  bool ShouldDropPacket(const Packet& packet) override;
  void SetLossRate(double drop_rate);
  static std::string GetName() { return "RandomPacketLoss"; }

 private:
  double drop_rate_;
};

class Transmission {
 public:
  Transmission() : connected_(false) {}
  virtual void SendToNetwork(std::shared_ptr<Packet>) = 0;
  virtual void ReceiveFromNetwork(std::shared_ptr<Packet>) = 0;
  virtual void SwitchOn() { connected_.store(true); }
  virtual void SwitchOff() { connected_.store(false); }
  virtual bool IsConnected() const { return connected_.load(); }

  virtual void SetReceiver(std::shared_ptr<Device> receiver) {
    receiver_ = receiver;
  }
  virtual std::shared_ptr<Device> GetReceiver() const { return receiver_; }

 protected:
  std::atomic<bool> connected_;
  std::shared_ptr<Device> receiver_;
};

// CommonTransmission has three features:
// 1. random packet loss
// 2. constant delay
// 3. bandwidth bottleneck with a cached buffer
class CommonTransmission : public Transmission {
 public:
  CommonTransmission(std::unique_ptr<PacketLoss> packet_loss, TimeDelta delay,
                     DataRate bandwidth, DataSize buffer_size);
  void SendToNetwork(std::shared_ptr<Packet> packet) override;
  void InFlight(std::shared_ptr<Packet> packet);
  void ReceiveFromNetwork(std::shared_ptr<Packet> packet) override;

  void SetDelay(TimeDelta delay);
  void SetPacketLoss(std::unique_ptr<PacketLoss> packet_loss);

  // Note that the new bandwidth will only take effect after
  // the current packets in flight are received.
  void SetBottleneckBandwidth(DataRate bandwidth);
  // Note that the new buffer size will only take effect after
  // the current packets in flight are received.
  void SetBottleneckBufferSize(DataSize buffer_size);

 private:
  std::unique_ptr<PacketLoss> packet_loss_;
  TimeDelta delay_;
  DataRate bottleneck_bandwidth_;
  DataSize bottleneck_buffer_size_;
  DataSize cached_buffer_size_;

  std::mutex delay_mutex_;
  std::mutex bottleneck_bandwidth_mutex_;
  std::mutex bottleneck_buffer_size_mutex_;
  std::mutex loss_mutex_;
};

}  // namespace araneid

#endif  // ARANEID_NETWORK_TRANSMISSION_HPP