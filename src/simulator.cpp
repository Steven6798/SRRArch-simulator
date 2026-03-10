/**
 * @file simulator.cpp
 * @brief Simulation engine implementation
 *
 * Core simulation logic including:
 * - ELF loading and memory initialization
 * - Instruction fetch from memory
 * - Execute with register updates
 * - Instruction tracing and logging
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "simulator.h"
#include "decoder.h"
#include "elf_loader.h"
#include "logger.h"
#include <cassert>
#include <iomanip>

Simulator::Simulator() = default;

Simulator::~Simulator() = default; // unique_ptr handles cleanup

bool Simulator::load_elf(const char *filename) {
  LOG_INFO("Loading ELF into simulator: %s", filename);

  // Create and use ElfLoader to parse the ELF
  loader = std::make_unique<ElfLoader>();

  if (!loader->load(filename)) {
    LOG_ERROR("Failed to load ELF file: %s", filename);
    loader.reset();
    return false;
  }

  // Get entry point
  entry_point = reinterpret_cast<uint64_t>(loader->get_entry_point());
  regs.set_pc(entry_point);
  LOG_INFO("Entry point set to 0x%lx", entry_point);

  // Load executable sections into memory
  const auto &sections = loader->get_executable_sections();
  size_t total_bytes = 0;

  for (const auto &sec : sections) {
    LOG_INFO("Loading section %s at 0x%lx (size: 0x%lx bytes)",
             sec.name.c_str(), sec.addr, sec.size);

    // Copy section data into simulator memory
    for (uint64_t i = 0; i < sec.size; i++) {
      memory[sec.addr + i] = sec.data[i];
    }
    total_bytes += sec.size;
  }

  LOG_INFO("Loaded %zu bytes into memory", total_bytes);

  // Initialize stack pointer (typical location - can be adjusted)
  regs.set_sp(0x80000000);
  LOG_DEBUG("Stack pointer initialized to 0x%lx", regs.get_sp());

  return true;
}

uint64_t Simulator::fetch() {
  uint64_t pc = regs.get_pc();
  uint64_t instruction = 0;

  // Fetch 8 bytes from memory at PC
  for (uint64_t i = 0; i < 8; i++) {
    uint64_t addr = pc + i;
    auto it = memory.find(addr);
    if (it != memory.end()) {
      instruction |= static_cast<uint64_t>(it->second) << (i * 8);
    } else {
      LOG_ERROR("Memory access violation at 0x%lx (PC=0x%lx)", addr, pc);
      running = false;
      return 0;
    }
  }

  LOG_DEBUG("Fetched instruction at 0x%lx: 0x%016lx", pc, instruction);

  // Advance PC
  regs.set_pc(pc + 8);

  return instruction;
}

void Simulator::execute(uint64_t instruction) {
  Opcode opcode = static_cast<Opcode>(instruction & 0xFF);

  // Log the instruction we're executing
  LOG_INFO("[%6lu] PC=0x%lx", instruction_count, regs.get_pc() - 8);

  // Decode and print (reuse your decoder)
  uint8_t bytes[8];
  for (int i = 0; i < 8; i++) {
    bytes[i] = (instruction >> (i * 8)) & 0xFF;
  }
  decode_instruction(bytes);

  switch (opcode) {
  case Opcode::NOP:
    exec_nop();
    break;

  case Opcode::RETURN: {
    uint8_t reg = (instruction >> 8) & 0x1F;
    exec_return(reg);
    break;
  }

  case Opcode::GENINT: {
    uint8_t reg = (instruction >> 8) & 0x1F;
    uint32_t imm = (instruction >> 13) & 0xFFFFFFFF;
    exec_genint(reg, imm);
    break;
  }

  case Opcode::SHL: {
    uint8_t dest = (instruction >> 8) & 0x1F;
    uint8_t src1 = (instruction >> 13) & 0x1F;
    uint8_t src2 = (instruction >> 18) & 0x1F;
    exec_shl(dest, src1, src2);
    break;
  }

  case Opcode::OR: {
    uint8_t dest = (instruction >> 8) & 0x1F;
    uint8_t src1 = (instruction >> 13) & 0x1F;
    uint8_t src2 = (instruction >> 18) & 0x1F;
    exec_or(dest, src1, src2);
    break;
  }

  case Opcode::MOV: {
    uint8_t dest = (instruction >> 8) & 0x1F;
    uint8_t src = (instruction >> 13) & 0x1F;
    exec_mov(dest, src);
    break;
  }

  default:
    LOG_ERROR("Unknown opcode 0x%02x at PC=0x%lx", static_cast<uint8_t>(opcode),
              regs.get_pc() - 8);
    running = false;
    break;
  }

  instruction_count++;
}

void Simulator::step() {
  if (!running)
    return;

  uint64_t instruction = fetch();
  if (running) {
    execute(instruction);
  }
}

void Simulator::run() {
  running = true;
  instruction_count = 0;

  LOG_INFO("=== Starting simulation ===");
  LOG_INFO("Entry point: 0x%lx", entry_point);
  LOG_DEBUG("Initial register state:");
  regs.dump();

  while (running) {
    step();

    // Optional: add a break condition for infinite loops
    if (instruction_count > 1000) {
      LOG_WARN("Reached max instruction count (1000)");
      running = false;
    }
  }

  LOG_INFO("=== Simulation finished ===");
  LOG_INFO("Executed %lu instructions", instruction_count);
  LOG_DEBUG("Final register state:");
  regs.dump();
}

// Memory access methods
uint64_t Simulator::read_mem(uint64_t addr, size_t size) {
  uint64_t value = 0;
  for (size_t i = 0; i < size; i++) {
    auto it = memory.find(addr + i);
    if (it != memory.end()) {
      value |= static_cast<uint64_t>(it->second) << (i * 8);
    } else {
      LOG_ERROR("Read from unmapped address 0x%lx", addr + i);
      return 0;
    }
  }
  LOG_DEBUG("Read 0x%lx from 0x%lx (size: %zu)", value, addr, size);
  return value;
}

void Simulator::write_mem(uint64_t addr, uint64_t value, size_t size) {
  LOG_DEBUG("Writing 0x%lx to 0x%lx (size: %zu)", value, addr, size);
  for (size_t i = 0; i < size; i++) {
    memory[addr + i] = (value >> (i * 8)) & 0xFF;
  }
}

// Instruction implementations
void Simulator::exec_nop() { LOG_DEBUG("  -> NOP"); }

void Simulator::exec_return(uint8_t reg) {
  uint64_t retval = regs.read(reg);
  regs.set_retval(retval);
  LOG_INFO("  -> RETURN: halting with return value %lu", retval);
  running = false;
}

void Simulator::exec_genint(uint8_t reg, uint32_t imm) {
  LOG_INFO("  -> GENINT: r%u = 0x%x", reg, imm);
  regs.write(reg, static_cast<uint64_t>(imm));
}

void Simulator::exec_shl(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t val1 = regs.read(src1);
  uint64_t val2 = regs.read(src2);
  uint64_t result = val1 << val2;

  LOG_INFO("  -> SHL: r%u = r%u << r%u (0x%lx << %lu = 0x%lx)", dest, src1,
           src2, val1, val2, result);

  regs.write(dest, result);
}

void Simulator::exec_or(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t val1 = regs.read(src1);
  uint64_t val2 = regs.read(src2);
  uint64_t result = val1 | val2;

  LOG_INFO("  -> OR: r%u = r%u | r%u (0x%lx | 0x%lx = 0x%lx)", dest, src1, src2,
           val1, val2, result);

  regs.write(dest, result);
}

void Simulator::exec_mov(uint8_t dest, uint8_t src) {
  uint64_t val = regs.read(src);
  LOG_INFO("  -> MOV: r%u = r%u (0x%lx)", dest, src, val);
  regs.write(dest, val);
}