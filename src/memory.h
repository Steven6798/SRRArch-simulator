/**
 * @file memory.h
 * @brief Memory model for SRRArch simulator
 *
 * Provides a flat byte-addressable memory space with
 * read/write operations and segment loading capabilities.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <map>
#include <vector>

namespace srrarch {

class Memory {
public:
  Memory() = default;
  ~Memory() = default;

  // Disable copy (can be enabled if needed)
  Memory(const Memory &) = delete;
  Memory &operator=(const Memory &) = delete;

  // Allow move
  Memory(Memory &&) = default;
  Memory &operator=(Memory &&) = default;

  // Load a segment of data into memory
  bool load_segment(uint64_t addr, const uint8_t *data, size_t size);

  // Read operations
  uint8_t read_byte(uint64_t addr) const;
  uint16_t read_word(uint64_t addr) const;
  uint32_t read_dword(uint64_t addr) const;
  uint64_t read_qword(uint64_t addr) const;

  // Generic read (size in bytes, 1-8)
  uint64_t read(uint64_t addr, size_t size) const;

  // Write operations
  void write_byte(uint64_t addr, uint8_t value);
  void write_word(uint64_t addr, uint16_t value);
  void write_dword(uint64_t addr, uint32_t value);
  void write_qword(uint64_t addr, uint64_t value);

  // Generic write (size in bytes, 1-8)
  void write(uint64_t addr, uint64_t value, size_t size);

  // Query
  bool is_mapped(uint64_t addr) const;
  size_t total_bytes() const { return data.size(); }

  // Debug
  void dump_segment(uint64_t start, uint64_t end) const;
  void dump() const;

private:
  std::map<uint64_t, uint8_t> data;

  // Helper for bounds checking
  bool check_range(uint64_t addr, size_t size) const;
};

} // namespace srrarch

#endif // MEMORY_H