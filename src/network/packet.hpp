#ifndef ARANEID_NETWORK_PACKET_HPP
#define ARANEID_NETWORK_PACKET_HPP

#include <cstdlib>
#include <deque>

namespace araneid {

struct Chunk {
  size_t size;
  size_t offset_written_start;
  size_t offset_written_end;
  size_t references_;
  uint8_t* data;
};

class Buffer {
 public:
  Buffer();
  ~Buffer();
  static Chunk* Allocate(size_t size);
  static Chunk* AllocateNew(size_t size);
  static void Recycle(Chunk* chunk);
  static void Destroy(Chunk* chunk);

  Buffer& operator=(const Buffer&);
  void Initialize();

 private:
  using FreeChunks = std::deque<Chunk*>;
  static std::unique_ptr<FreeChunks> free_chunks_;
  static size_t recycled_chunk_size_;
  Chunk* data_;
};

class Packet{

  private:
    Buffer buffer_;
};

}  // namespace araneid

#endif