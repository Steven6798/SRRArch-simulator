/**
 * @file memory.cpp
 * @brief Memory model implementation
 *
 * Implements byte-addressable memory using fast arrays for
 * stack and heap regions with unordered_map fallback for sparse
 * storage.Supports read/write operations of various sizes and segment loading
 * from ELF files
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "memory.h"
#include "logger.h"
#include <cstring>
#include <iomanip>
#include <iostream>

namespace srrarch {

Memory::Memory()
    : stack(std::make_unique<uint8_t[]>(STACK_SIZE)),
      heap(std::make_unique<uint8_t[]>(HEAP_SIZE)) {
  // Initialize fast regions to zero
  std::memset(stack.get(), 0, STACK_SIZE);
  std::memset(heap.get(), 0, HEAP_SIZE);
}

bool Memory::load_segment(uint64_t addr, const uint8_t *src_data, size_t size) {
  LOG_DBG("Loading segment at 0x%lx (size: %zu bytes)", addr, size);

  for (size_t i = 0; i < size; i++) {
    write_byte(addr + i, src_data[i]);
  }

  return true;
}

// Region check helpers
inline bool Memory::in_stack(uint64_t addr, size_t size) const {
  return addr >= STACK_START && addr + size <= STACK_START + STACK_SIZE;
}

inline bool Memory::in_heap(uint64_t addr, size_t size) const {
  return addr >= HEAP_START && addr + size <= HEAP_START + HEAP_SIZE;
}

// Slow path for sparse reads
template <typename T> T Memory::read_slow(uint64_t addr) const {
  T value = 0;
  for (size_t i = 0; i < sizeof(T); i++) {
    auto it = sparse.find(addr + i);
    if (it == sparse.end()) {
      LOG_ERROR("Read from unmapped address 0x%lx", addr + i);
      return 0;
    }
    // Cast to uint64_t first to avoid shift warnings
    value |= static_cast<T>(static_cast<uint64_t>(it->second) << (i * 8));
  }
  return value;
}

template <typename T> void Memory::write_slow(uint64_t addr, T value) {
  for (size_t i = 0; i < sizeof(T); i++) {
    sparse[addr + i] = static_cast<uint8_t>((value >> (i * 8)) & 0xFF);
  }
}

// Unified read implementation
template <typename T> T Memory::read_impl(uint64_t addr) const {
  if (in_stack(addr, sizeof(T))) {
    T value;
    std::memcpy(&value, stack.get() + (addr - STACK_START), sizeof(T));
    return value;
  }
  if (in_heap(addr, sizeof(T))) {
    T value;
    std::memcpy(&value, heap.get() + (addr - HEAP_START), sizeof(T));
    return value;
  }
  return read_slow<T>(addr);
}

// Unified write implementation
template <typename T> void Memory::write_impl(uint64_t addr, T value) {
  if (in_stack(addr, sizeof(T))) {
    std::memcpy(stack.get() + (addr - STACK_START), &value, sizeof(T));
    return;
  }
  if (in_heap(addr, sizeof(T))) {
    std::memcpy(heap.get() + (addr - HEAP_START), &value, sizeof(T));
    return;
  }
  write_slow<T>(addr, value);
}

// Explicit template instantiations
template uint8_t Memory::read_impl<uint8_t>(uint64_t) const;
template uint16_t Memory::read_impl<uint16_t>(uint64_t) const;
template uint32_t Memory::read_impl<uint32_t>(uint64_t) const;
template uint64_t Memory::read_impl<uint64_t>(uint64_t) const;

template void Memory::write_impl<uint8_t>(uint64_t, uint8_t);
template void Memory::write_impl<uint16_t>(uint64_t, uint16_t);
template void Memory::write_impl<uint32_t>(uint64_t, uint32_t);
template void Memory::write_impl<uint64_t>(uint64_t, uint64_t);

// Public wrapper methods
uint8_t Memory::read_byte(uint64_t addr) const {
  return read_impl<uint8_t>(addr);
}

uint16_t Memory::read_word(uint64_t addr) const {
  return read_impl<uint16_t>(addr);
}

uint32_t Memory::read_dword(uint64_t addr) const {
  return read_impl<uint32_t>(addr);
}

uint64_t Memory::read_qword(uint64_t addr) const {
  return read_impl<uint64_t>(addr);
}

uint64_t Memory::read(uint64_t addr, size_t size) const {
  if (size == 8)
    return read_qword(addr);
  if (size == 4)
    return read_dword(addr);
  if (size == 2)
    return read_word(addr);
  if (size == 1)
    return read_byte(addr);
  LOG_ERROR("Invalid read size: %zu", size);
  return 0;
}

void Memory::write_byte(uint64_t addr, uint8_t value) {
  write_impl<uint8_t>(addr, value);
}

void Memory::write_word(uint64_t addr, uint16_t value) {
  write_impl<uint16_t>(addr, value);
}

void Memory::write_dword(uint64_t addr, uint32_t value) {
  write_impl<uint32_t>(addr, value);
}

void Memory::write_qword(uint64_t addr, uint64_t value) {
  write_impl<uint64_t>(addr, value);
}

void Memory::write(uint64_t addr, uint64_t value, size_t size) {
  if (size == 8) {
    write_qword(addr, value);
    return;
  }
  if (size == 4) {
    write_dword(addr, value & 0xFFFFFFFF);
    return;
  }
  if (size == 2) {
    write_word(addr, value & 0xFFFF);
    return;
  }
  if (size == 1) {
    write_byte(addr, value & 0xFF);
    return;
  }
  LOG_ERROR("Invalid write size: %zu", size);
}

bool Memory::is_mapped(uint64_t addr) const {
  if (in_stack(addr))
    return true;
  if (in_heap(addr))
    return true;
  return sparse.find(addr) != sparse.end();
}

size_t Memory::total_bytes() const {
  return sparse.size() + STACK_SIZE + HEAP_SIZE;
}

void Memory::dump_segment(uint64_t start, uint64_t end) const {
  std::cout << "\nMemory dump 0x" << std::hex << start << " - 0x" << end
            << std::dec << "\n";
  std::cout << "================\n";

  for (uint64_t addr = start; addr <= end; addr += 16) {
    std::cout << std::hex << std::setw(8) << std::setfill('0') << addr << ": ";

    for (int i = 0; i < 16; i++) {
      uint64_t current_addr = addr + static_cast<uint64_t>(i);
      uint8_t byte = 0;

      // Get byte from appropriate region
      if (current_addr >= STACK_START &&
          current_addr < STACK_START + STACK_SIZE) {
        byte = stack[current_addr - STACK_START];
      } else if (current_addr >= HEAP_START &&
                 current_addr < HEAP_START + HEAP_SIZE) {
        byte = heap[current_addr - HEAP_START];
      } else {
        auto it = sparse.find(current_addr);
        if (it != sparse.end()) {
          byte = it->second;
        } else {
          std::cout << "?? ";
          if (i == 7)
            std::cout << " ";
          continue;
        }
      }

      std::cout << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(byte) << " ";
      if (i == 7)
        std::cout << " ";
    }

    std::cout << std::dec << std::setfill(' ') << std::endl;
  }
}

void Memory::dump() const {
  if (sparse.empty()) {
    // Check if stack or heap have any non-zero values
    bool has_data = false;
    for (size_t i = 0; i < STACK_SIZE && !has_data; i++) {
      if (stack[i] != 0)
        has_data = true;
    }
    for (size_t i = 0; i < HEAP_SIZE && !has_data; i++) {
      if (heap[i] != 0)
        has_data = true;
    }

    if (!has_data) {
      std::cout << "Memory is empty\n";
      return;
    }
  }

  // Dump a reasonable range
  dump_segment(HEAP_START, HEAP_START + HEAP_SIZE);
  dump_segment(STACK_START - 0x1000, STACK_START + 0x1000);
}

} // namespace srrarch