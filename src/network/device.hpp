#ifndef ARANEID_NETWORK_INTERNET_HPP
#define ARANEID_NETWORK_INTERNET_HPP
#include <unordered_map>

#include "base/log.hpp"
#include "system/bridge.hpp"

namespace araneid {

using Ipv4Address = std::string;  // IPv4 address in string format

class Transmission;
class Packet;
// Real network device should manage addresses, routes, and other
// network-related properties. But all in all, it's core functionality is to
// decide where to send packets and how to receive them. We use IP addresses to
// identify devices on the network.
class Device {
 public:
  // Send a packet to channel, which is identified by the destination address.
  virtual void Send(std::shared_ptr<Packet> packet) = 0;
  // For channel to invoke when a packet arrives.
  virtual void Receive(std::shared_ptr<Packet> packet) = 0;
  // Add a transmission channel to the device.
  virtual void AddTransmission(const Ipv4Address& address,
                               std::shared_ptr<Transmission> transmission) = 0;

 protected:
  // Decide which channel to send the packet to by the destination address.
  std::unordered_map<Ipv4Address, std::shared_ptr<Transmission>> out_goings_;
};

class CommonDevice : public Device {
 public:
  CommonDevice() = default;
  void Send(std::shared_ptr<Packet> packet) override;
  void Receive(std::shared_ptr<Packet> packet) override;
  void AddTransmission(const Ipv4Address& address,
                       std::shared_ptr<Transmission> transmission) override {
    out_goings_[address] = transmission;
  }

 private:
  std::shared_ptr<Bridge> bridge_;
};

}  // namespace araneid

#endif  // ARANEID_NETWORK_INTERNET_HPP