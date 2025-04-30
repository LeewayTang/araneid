#ifndef ARANEID_NETWORK_TRANSMISSION_HPP
#define ARANEID_NETWORK_TRANSMISSION_HPP

#include <queue>

#include "base/units.hpp"
#include "packet.hpp"

namespace araneid {

class PacketLoss {
 public:
  // This class is responsible for determining whether a packet should be
  // dropped
  virtual bool ShouldDropPacket(const Packet& packet) = 0;
};

class RandomPacketLoss : public PacketLoss {
 public:
  RandomPacketLoss(double drop_rate);
  bool ShouldDropPacket(const Packet& packet) override;

 private:
  double drop_rate_;
};

class Transmission {
 public:
  virtual void SendToNetwork(std::shared_ptr<const Packet>) = 0;
  virtual void ReceiveFromNetwork(std::shared_ptr<const Packet>) = 0;

 private:
  std::mutex mutex_;  // GET and SET of every member variable should be
                      // protected by this mutex
};

class NormalTransmission : public Transmission {
 public:
  void SendToNetwork(std::shared_ptr<const Packet>) override;
  void ReceiveFromNetwork(std::shared_ptr<const Packet>) override;

 private:
  std::unique_ptr<PacketLoss> packet_loss_;
  TimeDelta delay_;
  DataRate bottleneck_bandwidth_;
  DataSize bottleneck_buffer_size_;
  std::queue<std::shared_ptr<const Packet>> packet_queue_;
};

}  // namespace araneid

#endif