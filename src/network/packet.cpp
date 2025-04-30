#include "packet.hpp"

#include "base/log.hpp"

namespace araneid {

constexpr size_t kAllocatedPaddingBytes = 50;
constexpr size_t kMaxReservedChunkSize = 1000;

Buffer::Buffer() {
  if (free_chunks_ == nullptr) {
    recycled_chunk_size_ = 0;
    free_chunks_ = std::make_unique<FreeChunks>();
  }
}

Buffer::~Buffer() {
  data_->references_--;
  if (data_->references_ == 0) {
  }
}

Chunk* Buffer::Allocate(size_t size) {
  while (!free_chunks_->empty()) {
    Chunk* chunk = free_chunks_->back();
    free_chunks_->pop_back();
    if (chunk->size >= size) {
      return chunk;
    }
    Destroy(chunk);
  }
  Chunk* chunk = AllocateNew(size);
  return chunk;
}

Chunk* Buffer::AllocateNew(size_t size) {
  if (size == 0) return nullptr;
  size += kAllocatedPaddingBytes;
  size_t allocated_size = size - 1 + sizeof(Chunk);
  uint8_t* buf = new uint8_t[allocated_size];
  Chunk* chunk = reinterpret_cast<Chunk*>(buf);
  chunk->size = size;
  return chunk;
}

void Buffer::Recycle(Chunk* chunk) {
  if (chunk->references_ > 0) {
    ALOG_ERROR << "Chunk has references: " << chunk->references_;
    return;
  }
  if (chunk->size < recycled_chunk_size_ || free_chunks_ == nullptr ||
      free_chunks_->size() > kMaxReservedChunkSize) {
    Destroy(chunk);
    return;
  }
  free_chunks_->push_back(chunk);
}

void Buffer::Destroy(Chunk* chunk) {
  if (chunk->references_ > 0) {
    ALOG_ERROR << "Chunk has references: " << chunk->references_;
    return;
  }
  uint8_t* useless = reinterpret_cast<uint8_t*>(chunk);
  delete[] useless;
}

}  // namespace araneid