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

bool Memory::check_range(uint64_t addr, size_t size) const {
  for (size_t i = 0; i < size; i++) {
    uint64_t current_addr = addr + i;

    // Fast path for stack region (always mapped)
    if (current_addr >= STACK_START &&
        current_addr < STACK_START + STACK_SIZE) {
      continue;
    }

    // Fast path for heap region (always mapped)
    if (current_addr >= HEAP_START && current_addr < HEAP_START + HEAP_SIZE) {
      continue;
    }

    // Check sparse map
    if (sparse.find(current_addr) == sparse.end()) {
      LOG_ERROR("Memory access violation at 0x%lx", current_addr);
      return false;
    }
  }
  return true;
}

uint8_t Memory::read_byte(uint64_t addr) const {
  // Fast path: stack region
  if (addr >= STACK_START && addr < STACK_START + STACK_SIZE) {
    uint64_t offset = addr - STACK_START;
    if (offset >= STACK_SIZE) {
      LOG_ERROR("Stack access out of bounds: 0x%lx", addr);
      return 0;
    }
    return stack.get()[offset];
  }

  // Fast path: heap region
  if (addr >= HEAP_START && addr < HEAP_START + HEAP_SIZE) {
    uint64_t offset = addr - HEAP_START;
    if (offset >= HEAP_SIZE) {
      LOG_ERROR("Heap access out of bounds: 0x%lx", addr);
      return 0;
    }
    return heap.get()[offset];
  }

  // Sparse region
  auto it = sparse.find(addr);
  if (it == sparse.end()) {
    LOG_ERROR("Read from unmapped address 0x%lx", addr);
    return 0;
  }
  return it->second;
}

void Memory::write_byte(uint64_t addr, uint8_t value) {
  // Fast path: stack region
  if (addr >= STACK_START && addr < STACK_START + STACK_SIZE) {
    uint64_t offset = addr - STACK_START;
    if (offset >= STACK_SIZE) {
      LOG_ERROR("Stack access out of bounds: 0x%lx", addr);
      return;
    }
    stack.get()[offset] = value;
    return;
  }

  // Fast path: heap region
  if (addr >= HEAP_START && addr < HEAP_START + HEAP_SIZE) {
    uint64_t offset = addr - HEAP_START;
    if (offset >= HEAP_SIZE) {
      LOG_ERROR("Heap access out of bounds: 0x%lx", addr);
      return;
    }
    heap.get()[offset] = value;
    return;
  }

  // Sparse region
  sparse[addr] = value;
}

uint16_t Memory::read_word(uint64_t addr) const {
  if (!check_range(addr, 2))
    return 0;

  uint16_t value = 0;
  for (size_t i = 0; i < 2; i++) {
    value |= static_cast<uint16_t>(read_byte(addr + i)) << (i * 8);
  }
  return value;
}

uint32_t Memory::read_dword(uint64_t addr) const {
  if (!check_range(addr, 4))
    return 0;

  uint32_t value = 0;
  for (size_t i = 0; i < 4; i++) {
    value |= static_cast<uint32_t>(read_byte(addr + i)) << (i * 8);
  }
  return value;
}

uint64_t Memory::read_qword(uint64_t addr) const {
  if (!check_range(addr, 8))
    return 0;

  uint64_t value = 0;
  for (size_t i = 0; i < 8; i++) {
    value |= static_cast<uint64_t>(read_byte(addr + i)) << (i * 8);
  }
  return value;
}

uint64_t Memory::read(uint64_t addr, size_t size) const {
  if (size > 8) {
    LOG_ERROR("Invalid read size: %zu (max 8)", size);
    return 0;
  }

  if (!check_range(addr, size))
    return 0;

  uint64_t value = 0;
  for (size_t i = 0; i < size; i++) {
    value |= static_cast<uint64_t>(read_byte(addr + i)) << (i * 8);
  }

  LOG_DBG("Read 0x%lx from 0x%lx (size: %zu)", value, addr, size);
  return value;
}

void Memory::write_word(uint64_t addr, uint16_t value) {
  LOG_DBG("Writing 0x%04x to 0x%lx", value, addr);
  for (size_t i = 0; i < 2; i++) {
    write_byte(addr + i, (value >> (i * 8)) & 0xFF);
  }
}

void Memory::write_dword(uint64_t addr, uint32_t value) {
  LOG_DBG("Writing 0x%08x to 0x%lx", value, addr);
  for (size_t i = 0; i < 4; i++) {
    write_byte(addr + i, (value >> (i * 8)) & 0xFF);
  }
}

void Memory::write_qword(uint64_t addr, uint64_t value) {
  LOG_DBG("Writing 0x%016lx to 0x%lx", value, addr);
  for (size_t i = 0; i < 8; i++) {
    write_byte(addr + i, (value >> (i * 8)) & 0xFF);
  }
}

void Memory::write(uint64_t addr, uint64_t value, size_t size) {
  if (size > 8) {
    LOG_ERROR("Invalid write size: %zu (max 8)", size);
    return;
  }

  LOG_DBG("Writing 0x%lx to 0x%lx (size: %zu)", value, addr, size);
  for (size_t i = 0; i < size; i++) {
    write_byte(addr + i, (value >> (i * 8)) & 0xFF);
  }
}

bool Memory::is_mapped(uint64_t addr) const {
  // Stack and heap are always mapped
  if (addr >= STACK_START && addr < STACK_START + STACK_SIZE)
    return true;
  if (addr >= HEAP_START && addr < HEAP_START + HEAP_SIZE)
    return true;

  // Check sparse map
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