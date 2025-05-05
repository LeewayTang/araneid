#include "bridge.hpp"

#include <fcntl.h>         // For O_RDWR
#include <linux/if_tun.h>  // For IFF_TAP and IFF_NO_PI
#include <net/if.h>        // For struct ifreq
#include <sys/ioctl.h>     // For ioctl
#include <unistd.h>        // For close()

#include <cstring>  // For strcpy

#include "base/log.hpp"
#include "network/packet.hpp"
namespace araneid {

TapBridge::TapBridge(std::string tap_device_name)
    : tap_device_name_(tap_device_name) {
  write_buffer_ = new uint8_t[65536];  // 64kB buffer
  sock_ = FindTapDeviceAndOpenSocket();
}

TapBridge::~TapBridge() {
  if (fd_reader_) {
    fd_reader_->Stop();
  }
  if (sock_ != -1) {
    close(sock_);
    sock_ = -1;
  }
  delete[] write_buffer_;
  write_buffer_ = nullptr;
}

int TapBridge::FindTapDeviceAndOpenSocket() {
  // Creating and managing the TAP device needs the help of TUN device.
  int tap_fd = open("/dev/net/tun", O_RDWR);
  if (tap_fd < 0) {
    ALOG_ERROR << "Failed to open /dev/net/tun";
  }

  // Get the TAP device, which is created before this program is run.
  struct ifreq ifr;
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  strcpy(ifr.ifr_name, tap_device_name_.c_str());
  if (ioctl(tap_fd, TUNSETIFF, &ifr) < 0) {
    ALOG_ERROR << "Failed to set TAP device flags";
  }
  tap_device_name_ = std::string((char *)ifr.ifr_name);
  ALOG_INFO << "TAP device name: " << tap_device_name_;
  return tap_fd;
}

void TapBridge::Start() {
  fd_reader_ = std::make_unique<FdReader>();
  fd_reader_->Start(sock_, std::shared_ptr<Bridge>(this));
}

void TapBridge::ForwardOut(uint8_t *data, size_t len) {
  // Create a packet from the data and send it to the bridged device.
  // This packet is completed with all headers, that is, the ethernet header.
  std::shared_ptr<Packet> packet = Packet::Create(data, DataSize::Bytes(len));
  std::free(data);
  data = nullptr;
  bridged_device_->Send(packet);
}

void TapBridge::ForwardIn(std::shared_ptr<Packet> packet) {
  // Bridged device sends a packet to the TAP device.
  if (!packet->CopyData(write_buffer_, packet->GetSize().Bytes())) {
    return;
  }
  ssize_t bytes_written =
      write(sock_, write_buffer_, packet->GetSize().Bytes());
}

}  // namespace araneid