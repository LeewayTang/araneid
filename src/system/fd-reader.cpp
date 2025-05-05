#include "fd-reader.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

#include "base/log.hpp"
#include "bridge.hpp"
#include "network/packet.hpp"

namespace araneid {

FdReader::FdReader() : fd_(-1), stop_(false) {
  event_pipe_[0] = -1;
  event_pipe_[1] = -1;
}

FdReader::~FdReader() { Stop(); }

void FdReader::Start(int fd, std::shared_ptr<Bridge> data_bridge) {
  fd_ = fd;
  data_bridge_ = data_bridge;

  int ret = pipe(event_pipe_);
  if (ret == -1) {
    ALOG_ERROR << "Failed to create pipe: " << std::strerror(errno);
  } else if (fcntl(event_pipe_[0], F_SETFL, ret | O_NONBLOCK) == -1) {
    ALOG_ERROR << "Failed to set pipe to non-blocking: "
               << std::strerror(errno);
  }
  read_thread_ = std::thread(&FdReader::Run, this);
}

void FdReader::Stop() {
  stop_.store(true);
  if (event_pipe_[1] != -1) {
    char sig = 0;
    ssize_t ret = write(event_pipe_[1], &sig, sizeof(sig));
    if (ret != sizeof(sig)) {
      ALOG_WARNING << "Incomplete write to event pipe: "
                   << std::strerror(errno);
    }
  }
  if (read_thread_.joinable()) {
    read_thread_.join();
  }
  // close the write end of the pipe
  if (event_pipe_[1] != -1) {
    close(event_pipe_[1]);
    event_pipe_[1] = -1;
  }
  // close the read end of the pipe
  if (event_pipe_[0] != -1) {
    close(event_pipe_[0]);
    event_pipe_[0] = -1;
  }
  fd_ = -1;
  data_bridge_ = nullptr;
  stop_.store(false);
}

FdReader::Data FdReader::Read() {
  uint8_t* buf = (uint8_t*)std::malloc(65536);
  if (!buf) {
    ALOG_ERROR << "Failed to allocate buffer for reading data";
  }
  ssize_t bytes = read(fd_, buf, 65536);
  if (bytes <= 0) {
    ALOG_INFO << "FdReader::Read() read done";
    std::free(buf);
    buf = nullptr;
    bytes = 0;
  }
  return Data(buf, bytes);
}

void FdReader::Run() {
  int nfds = 0;
  fd_set read_fds;
  nfds = (fd_ > event_pipe_[0] ? fd_ : event_pipe_[0]) + 1;
  FD_ZERO(&read_fds);
  FD_SET(fd_, &read_fds);
  FD_SET(event_pipe_[0], &read_fds);
  while (true) {
    fd_set readfds = read_fds;
    int ret = select(nfds, &readfds, nullptr, nullptr, nullptr);
    if (ret == -1 && errno != EINTR) {
      ALOG_ERROR << "Select error: " << std::strerror(errno);
    }
    if (FD_ISSET(event_pipe_[0], &readfds)) {
      while (true) {
        char buf[1024];
        ssize_t bytes = read(event_pipe_[0], buf, sizeof(buf));
        if (bytes == 0) {
          ALOG_ERROR << "Pipe closed";
          break;
        }
        if (bytes < 0) {
          if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
            break;
          } else {
            ALOG_ERROR << "Error reading from pipe: " << std::strerror(errno);
          }
        }
      }
    }
    if (stop_.load()) {
      break;
    }
    if (FD_ISSET(fd_, &readfds)) {
      Data data = Read();
      if (data.bytes == 0) {
        break;
      } else if (data.bytes > 0) {
        if (data_bridge_) {
          data_bridge_->ForwardIn(Packet::Create(data.buffer,DataSize::Bytes(data.bytes)));
        }
      }
    }
  }
}
}  // namespace araneid