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
#include "instruction.h"
#include "logger.h"
#include <cassert>
#include <iomanip>

namespace srrarch {

Simulator::Simulator() = default;
Simulator::~Simulator() = default;

bool Simulator::load_elf(const char *filename) {
  LOG_INFO("Loading ELF into simulator: %s", filename);

  // Create and use ElfLoader to parse the ELF
  loader = std::make_unique<ElfLoader>();

  LoadResult result = loader->load(filename);
  if (result != LoadResult::SUCCESS) {
    LOG_ERROR("Failed to load ELF file: %s - %s", filename,
              load_result_to_string(result));
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

    // Use Memory class to load the segment
    memory.load_segment(sec.addr, sec.data, sec.size);
    total_bytes += sec.size;
  }

  LOG_INFO("Loaded %zu bytes into memory", total_bytes);

  // Initialize stack pointer
  regs.set_sp(0x80000000);
  LOG_DEBUG("Stack pointer initialized to 0x%lx", regs.get_sp());

  return true;
}

uint64_t Simulator::fetch() {
  uint64_t pc = regs.get_pc();

  // Use Memory class to read instruction
  uint64_t instruction = memory.read_qword(pc);

  LOG_DEBUG("Fetched instruction at 0x%lx: 0x%016lx", pc, instruction);

  // Advance PC
  regs.set_pc(pc + 8);

  return instruction;
}

void Simulator::execute(uint64_t raw_instruction) {
  Instruction inst(raw_instruction);

  LOG_INFO("[%6lu] PC=0x%lx", instruction_count, regs.get_pc() - 8);

  // Decode and print using Instruction class
  inst.print_bytes();
  inst.print();

  switch (inst.opcode()) {
  case Opcode::NOP:
    exec_nop();
    break;

  case Opcode::RETURN:
    exec_return(inst.return_reg());
    break;

  case Opcode::GENINT:
    exec_genint(inst.genint_reg(), inst.immediate());
    break;

  case Opcode::SHL:
    exec_shl(inst.shl_dest(), inst.shl_src1(), inst.shl_src2());
    break;

  case Opcode::OR:
    exec_or(inst.or_dest(), inst.or_src1(), inst.or_src2());
    break;

  case Opcode::MOV:
    exec_mov(inst.dest_reg(), inst.src_reg());
    break;

  default:
    LOG_ERROR("Unknown opcode at PC=0x%lx", regs.get_pc() - 8);
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

} // namespace srrarch