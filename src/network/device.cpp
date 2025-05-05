#include "device.hpp"

#include "packet.hpp"
#include "transmission.hpp"

namespace araneid {
void CommonDevice::Send(std::shared_ptr<Packet> packet) {
  if (packet == nullptr) {
    ALOG_ERROR << "Packet is null";
    return;
  }
  auto it = out_goings_.find(packet->GetDstIpv4());
  if (it != out_goings_.end()) {
    it->second->SendToNetwork(packet);
  } else {
    ALOG_ERROR << "No outgoing transmission for packet's IP address.";
  }
}

void CommonDevice::Receive(std::shared_ptr<Packet> packet) {
  if (packet == nullptr) {
    ALOG_ERROR << "Packet is null";
  }
  if (bridge_ != nullptr) {
    bridge_->ForwardIn(packet);
  } else {
    ALOG_ERROR << "No bridge to forward the packet.";
  }
}

}  // namespace araneid