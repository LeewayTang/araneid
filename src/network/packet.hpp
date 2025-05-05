#ifndef ARANEID_NETWORK_PACKET_HPP
#define ARANEID_NETWORK_PACKET_HPP

#include <base/units.hpp>
#include <cstdlib>
#include <deque>
#include <memory>

#include "device.hpp"

namespace araneid {
struct Chunk {
  size_t size;
  size_t references_;
  uint8_t* data;
};

class Buffer {
 public:
  Buffer(DataSize size);
  ~Buffer();
  static Chunk* Allocate(size_t size);
  static Chunk* AllocateNew(size_t size);
  static void Recycle(Chunk* chunk);
  static void Destroy(Chunk* chunk);

  Buffer& operator=(const Buffer&);

  // copy data from the given buffer
  void Write(const uint8_t* data, DataSize size);

  bool Copy(uint8_t* data, size_t size) const;

 private:
  using FreeChunks = std::deque<Chunk*>;
  static std::unique_ptr<FreeChunks> free_chunks_;
  // The maximum size of the recycled chunk, and chunks smaller than this
  // won't be recycled. This is a performance optimization to avoid allocating
  // too many small chunks.
  static size_t recycled_chunk_size_;
  Chunk* data_;
};

class Packet {
 public:
  // Create a packet by copying the data from the given buffer,
  // so the data is not owned by the packet, remember to free it.
  static std::shared_ptr<Packet> Create(const uint8_t* data, DataSize size) {
    return std::make_shared<Packet>(data, size);
  }
  DataSize GetSize() const { return packet_size_; }
  Ipv4Address GetSrcIpv4() const { return src_; }
  Ipv4Address GetDstIpv4() const { return dst_; }
  // Copy the data from the packet to the given buffer, return the copied size.
  bool CopyData(uint8_t* data, size_t size) const;
  Packet(const uint8_t* data, DataSize size);

 private:
  Ipv4Address src_;
  Ipv4Address dst_;
  Buffer buffer_;
  DataSize packet_size_;
};

}  // namespace araneid

#endif  // ARANEID_NETWORK_PACKET_HPP