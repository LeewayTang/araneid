#ifndef ARANEID_SYSTEM_TAP_BRIDGE_HPP
#define ARANEID_SYSTEM_TAP_BRIDGE_HPP

#include <cstdint>
#include <cstdlib>

#include "fd-reader.hpp"
#include "network/device.hpp"

namespace araneid {
class Packet;

class Bridge {
 public:
  virtual void ForwardOut(uint8_t *data, size_t len) = 0;
  virtual void ForwardIn(std::shared_ptr<Packet> packet) = 0;
};

class Device;
class TapBridge : public Bridge {
 public:
  TapBridge(std::string tap_device_name);
  ~TapBridge();
  // Deliver the packet to the bridged device, and it will decide where to send
  // it.
  void ForwardOut(uint8_t *data, size_t len) override;
  // We make sure that the TAP device is in promiscuous mode, so the MAC address
  // is not important. Just deliver the packet to TAP device.
  void ForwardIn(std::shared_ptr<Packet> packet) override;

  void Start();

 private:
  int FindTapDeviceAndOpenSocket();
  int sock_;
  uint8_t *write_buffer_;
  std::string tap_device_name_;
  std::unique_ptr<FdReader> fd_reader_;
  std::shared_ptr<Device> bridged_device_;
};
}  // namespace araneid

#endif  // ARANEID_SYSTEM_TAP_BRIDGE_HPP