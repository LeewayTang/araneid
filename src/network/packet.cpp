#include "packet.hpp"

#include <arpa/inet.h>   // only for UNIX-like systems
#include <netinet/ip.h>  // for struct iphdr

#include <cstring>

#include "base/log.hpp"
namespace araneid {

constexpr size_t kAllocatedPaddingBytes = 50;
constexpr size_t kMaxReservedChunkSize = 1000;

Buffer::Buffer(DataSize size) {
  if (free_chunks_ == nullptr) {
    recycled_chunk_size_ = 0;
    free_chunks_ = std::make_unique<FreeChunks>();
  }
  data_ = Allocate(size.Bytes());
}

Buffer::~Buffer() {
  data_->references_--;
  if (data_->references_ == 0) {
    Recycle(data_);
  }
}

Buffer& Buffer::operator=(const Buffer& other) {
  if (this != &other) {
    data_->references_--;
    if (data_->references_ == 0) {
      Recycle(data_);
    }
    data_ = other.data_;
    data_->references_++;
  }
  return *this;
}

void Buffer::Write(const uint8_t* data, DataSize size) {
  if (data_ == nullptr) {
    ALOG_ERROR << "Buffer is null";
    return;
  }
  if (data_->size < size.Bytes()) {
    ALOG_ERROR << "Buffer size is less than data size";
    return;
  }
  std::memcpy(data_->data, data, size.Bytes());
}

bool Buffer::Copy(uint8_t* data, size_t size) const {
  if (data_ == nullptr) {
    ALOG_ERROR << "Buffer is null";
    return false;
  }
  if (data_->size < size) {
    ALOG_ERROR << "Buffer size is less than data size";
    return false;
  }
  std::memcpy(data, data_->data, size);
  return true;
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
  recycled_chunk_size_ = std::max(recycled_chunk_size_, chunk->size);
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

Packet::Packet(const uint8_t* data, DataSize size)
    : buffer_(size), packet_size_(size) {
  if (data == nullptr || size.Bytes() == 0) {
    ALOG_ERROR << "Invalid data or size";
    return;
  }
  buffer_.Write(data, size);

  // Minimum ethernet frame size is 14 bytes
  // (6 bytes destination MAC, 6 bytes source MAC, 2 bytes type)
  if (size.Bytes() < 14) {
    ALOG_ERROR << "Invalid packet size: " << size.Bytes() << " bytes";
    return;
  }

  // parse ethernet header
  uint16_t eth_type = ntohs(*reinterpret_cast<const uint16_t*>(data + 12));
  const uint8_t* network_layer = data + 14;
  size_t remaining = size.Bytes() - 14;

  if (eth_type == 0x8100) {  // 802.1Q VLAN
    if (remaining < 4) {
      ALOG_ERROR << "Invalid packet size: " << size.Bytes() << " bytes";
    }
    eth_type = ntohs(*reinterpret_cast<const uint16_t*>(network_layer + 2));
    network_layer += 4;  // skip the VLAN header
    remaining -= 4;
  }

  // recognize IP layer
  if (eth_type == 0x0800) {  // IPv4
    if (remaining < 20) {
      ALOG_ERROR << "Invalid packet size: " << size.Bytes() << " bytes";
    }
    const struct iphdr* ip_header =
        reinterpret_cast<const struct iphdr*>(network_layer);

    if (ip_header->version != 4) {
      ALOG_ERROR << "Invalid IP version: " << ip_header->version;
    }
    size_t ip_header_length = ip_header->ihl * 4;
    if (ip_header_length < 20 || ip_header_length > remaining) {
      ALOG_ERROR << "Invalid IP header length: " << ip_header_length;
    }
    std::string src_ip(inet_ntoa(*(struct in_addr*)&ip_header->saddr));
    std::string dst_ip(inet_ntoa(*(struct in_addr*)&ip_header->daddr));
    src_ = Ipv4Address(src_ip);
    dst_ = Ipv4Address(dst_ip);
  } else if (eth_type == 0x86dd) {  // IPv6
    ALOG_ERROR << "IPv6 is not supported yet";
  } else {
    ALOG_ERROR << "Unknown ethernet type: " << std::hex << eth_type;
  }
}

bool Packet::CopyData(uint8_t* data, size_t size) const {
  return buffer_.Copy(data, size);
}

}  // namespace araneid