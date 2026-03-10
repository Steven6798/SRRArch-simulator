/**
 * @file memory.cpp
 * @brief Memory model implementation
 *
 * Implements byte-addressable memory using std::map for sparse
 * storage. Supports read/write operations of various sizes and
 * segment loading from ELF files.
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

bool Memory::load_segment(uint64_t addr, const uint8_t *src_data, size_t size) {
  LOG_DEBUG("Loading segment at 0x%lx (size: %zu bytes)", addr, size);

  for (size_t i = 0; i < size; i++) {
    this->data[addr + i] = src_data[i];
  }

  return true;
}

bool Memory::check_range(uint64_t addr, size_t size) const {
  for (size_t i = 0; i < size; i++) {
    if (data.find(addr + i) == data.end()) {
      LOG_ERROR("Memory access violation at 0x%lx", addr + i);
      return false;
    }
  }
  return true;
}

uint8_t Memory::read_byte(uint64_t addr) const {
  auto it = data.find(addr);
  if (it == data.end()) {
    LOG_ERROR("Read from unmapped address 0x%lx", addr);
    return 0;
  }
  return it->second;
}

uint16_t Memory::read_word(uint64_t addr) const {
  if (!check_range(addr, 2))
    return 0;

  uint16_t value = 0;
  for (size_t i = 0; i < 2; i++) {
    uint16_t byte = static_cast<uint16_t>(data.at(addr + i));
    value |= static_cast<uint16_t>(byte << (i * 8));
  }
  return value;
}

uint32_t Memory::read_dword(uint64_t addr) const {
  if (!check_range(addr, 4))
    return 0;

  uint32_t value = 0;
  for (size_t i = 0; i < 4; i++) {
    value |= static_cast<uint32_t>(data.at(addr + i)) << (i * 8);
  }
  return value;
}

uint64_t Memory::read_qword(uint64_t addr) const {
  if (!check_range(addr, 8))
    return 0;

  uint64_t value = 0;
  for (size_t i = 0; i < 8; i++) {
    value |= static_cast<uint64_t>(data.at(addr + i)) << (i * 8);
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
    value |= static_cast<uint64_t>(data.at(addr + i)) << (i * 8);
  }

  LOG_DEBUG("Read 0x%lx from 0x%lx (size: %zu)", value, addr, size);
  return value;
}

void Memory::write_byte(uint64_t addr, uint8_t value) {
  LOG_DEBUG("Writing 0x%02x to 0x%lx", value, addr);
  data[addr] = value;
}

void Memory::write_word(uint64_t addr, uint16_t value) {
  LOG_DEBUG("Writing 0x%04x to 0x%lx", value, addr);
  for (size_t i = 0; i < 2; i++) {
    uint8_t byte = static_cast<uint8_t>((value >> (i * 8)) & 0xFF);
    data[addr + i] = byte;
  }
}

void Memory::write_dword(uint64_t addr, uint32_t value) {
  LOG_DEBUG("Writing 0x%08x to 0x%lx", value, addr);
  for (size_t i = 0; i < 4; i++) {
    data[addr + i] = (value >> (i * 8)) & 0xFF;
  }
}

void Memory::write_qword(uint64_t addr, uint64_t value) {
  LOG_DEBUG("Writing 0x%016lx to 0x%lx", value, addr);
  for (size_t i = 0; i < 8; i++) {
    data[addr + i] = (value >> (i * 8)) & 0xFF;
  }
}

void Memory::write(uint64_t addr, uint64_t value, size_t size) {
  if (size > 8) {
    LOG_ERROR("Invalid write size: %zu (max 8)", size);
    return;
  }

  LOG_DEBUG("Writing 0x%lx to 0x%lx (size: %zu)", value, addr, size);
  for (size_t i = 0; i < size; i++) {
    data[addr + i] = (value >> (i * 8)) & 0xFF;
  }
}

bool Memory::is_mapped(uint64_t addr) const {
  return data.find(addr) != data.end();
}

void Memory::dump_segment(uint64_t start, uint64_t end) const {
  std::cout << "\nMemory dump 0x" << std::hex << start << " - 0x" << end
            << std::dec << "\n";
  std::cout << "================\n";

  for (uint64_t addr = start; addr <= end; addr += 16) {
    std::cout << std::hex << std::setw(8) << std::setfill('0') << addr << ": ";

    for (int i = 0; i < 16; i++) {
      uint64_t current_addr = addr + static_cast<uint64_t>(i);
      auto it = data.find(current_addr);
      if (it != data.end()) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(it->second) << " ";
      } else {
        std::cout << "?? ";
      }
      if (i == 7)
        std::cout << " ";
    }

    std::cout << std::dec << std::setfill(' ') << std::endl;
  }
}

void Memory::dump() const {
  if (data.empty()) {
    std::cout << "Memory is empty\n";
    return;
  }

  uint64_t start = data.begin()->first;
  uint64_t end = data.rbegin()->first;
  dump_segment(start, end);
}

} // namespace srrarch