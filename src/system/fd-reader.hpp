#ifndef ARANEID_SYSTEM_FD_READER_HPP
#define ARANEID_SYSTEM_FD_READER_HPP

#include <atomic>
#include <memory>
#include <thread>

namespace araneid {
class Bridge;
// Only for UNIX-like systems
class FdReader {
 public:
  FdReader();
  ~FdReader();
  void Start(int fd, std::shared_ptr<Bridge> data_bridge);
  void Stop();

  struct Data {
    uint8_t* buffer;
    size_t bytes;
    Data() : buffer(nullptr), bytes(0) {}
    Data(uint8_t* buf, size_t b) : buffer(buf), bytes(b) {}
  };

  Data Read();

 private:
  void Run();
  int fd_;
  int event_pipe_[2];                    // pipe to signal between threads
  std::atomic<bool> stop_;               // flag to stop the read thread
  std::shared_ptr<Bridge> data_bridge_;  // callback to invoke when we read data
  std::thread read_thread_;
};

}  // namespace araneid

#endif  // ARANEID_SYSTEM_FD_READER_HPP