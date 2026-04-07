/**
 * @file memory.h
 * @brief Memory model for SRRArch simulator
 *
 * Provides a flat byte-addressable memory space with
 * read/write operations and segment loading capabilities.
 * Optimized with fast paths for stack and heap regions.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace srrarch {

class Memory {
public:
  Memory();
  ~Memory() = default;

  // Disable copy
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
  uint64_t read(uint64_t addr, size_t size) const;

  // Write operations
  void write_byte(uint64_t addr, uint8_t value);
  void write_word(uint64_t addr, uint16_t value);
  void write_dword(uint64_t addr, uint32_t value);
  void write_qword(uint64_t addr, uint64_t value);
  void write(uint64_t addr, uint64_t value, size_t size);

  // Query
  bool is_mapped(uint64_t addr) const;
  size_t total_bytes() const;

  // Debug
  void dump_segment(uint64_t start, uint64_t end) const;
  void dump() const;

private:
  // Stack grows DOWNWARD from STACK_TOP to STACK_BOTTOM
  // Highest address (initial SP)
  static constexpr uint64_t STACK_TOP = 0x80000000;
  static constexpr uint64_t STACK_SIZE = 2 * 1024 * 1024; // 2MB stack
  // Lowest address
  static constexpr uint64_t STACK_BOTTOM = STACK_TOP - STACK_SIZE;

  // Heap grows UPWARD from HEAP_START to HEAP_END
  static constexpr uint64_t HEAP_START = 0x00010000;
  static constexpr uint64_t HEAP_SIZE = 32 * 1024 * 1024; // 32MB heap
  static constexpr uint64_t HEAP_END = HEAP_START + HEAP_SIZE;

  std::unique_ptr<uint8_t[]> stack;
  std::unique_ptr<uint8_t[]> heap;
  std::unordered_map<uint64_t, uint8_t> sparse;

  // Helper methods for region checks
  bool in_stack(uint64_t addr, size_t size = 1) const;
  bool in_heap(uint64_t addr, size_t size = 1) const;

  // Helper methods for offset calculation
  uint64_t stack_offset(uint64_t addr) const;
  uint64_t heap_offset(uint64_t addr) const;

  // Template implementations
  template <typename T> T read_impl(uint64_t addr) const;
  template <typename T> void write_impl(uint64_t addr, T value);

  // Slow path helpers
  template <typename T> T read_slow(uint64_t addr) const;
  template <typename T> void write_slow(uint64_t addr, T value);
};

} // namespace srrarch

#endif // MEMORY_H