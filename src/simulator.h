/**
 * @file simulator.h
 * @brief Main simulation engine for SRRArch
 *
 * Implements the fetch-decode-execute loop with memory model
 * and register file. Supports loading ELF files and executing
 * programs step-by-step or continuously.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "elf_loader.h"
#include "instruction.h"
#include "memory.h"
#include "registers.h"
#include <cstdint>
#include <memory>

namespace srrarch {

// Forward declaration
class ElfLoader;

class Simulator {
public:
  Simulator();
  ~Simulator();

  // Disable copy
  Simulator(const Simulator &) = delete;
  Simulator &operator=(const Simulator &) = delete;

  // Allow move
  Simulator(Simulator &&) = default;
  Simulator &operator=(Simulator &&) = default;

  LoadResult load_elf(const char *filename);

  // Run the program
  void run();

  // Single step execution (for debugging)
  void step();

  // Get register file (for inspection)
  const Registers &get_registers() const { return regs; }

  // Check if simulation should stop
  bool is_running() const { return running; }

  // Stop simulation
  void stop() { running = false; }

private:
  Registers regs;
  std::unique_ptr<ElfLoader> loader;
  Memory memory;
  uint64_t entry_point = 0;
  bool running = false;
  uint64_t instruction_count = 0;

  // Fetch instruction at current PC
  uint64_t fetch();

  // Execute a single instruction
  void execute(const Instruction &inst);

  // Instruction implementations
  void exec_nop();
  void exec_return(uint8_t reg);
  void exec_genint(uint8_t reg, uint32_t imm);
  void exec_mov(uint8_t dest, uint8_t src);

  // Arithmetic
  void exec_add(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_sub(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_mul(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_sdiv(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_udiv(uint8_t dest, uint8_t src1, uint8_t src2);

  // Logical
  void exec_and(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_or(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_xor(uint8_t dest, uint8_t src1, uint8_t src2);

  // Shifts
  void exec_shl(uint8_t dest, uint8_t src, uint8_t amount);
  void exec_sra(uint8_t dest, uint8_t src, uint8_t amount);
  void exec_srl(uint8_t dest, uint8_t src, uint8_t amount);

  // Comparisons (set dest to 1 or 0)
  void exec_cmpeq(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_cmpne(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_cmplt(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_cmpgt(uint8_t dest, uint8_t src1, uint8_t src2);

  // Memory operations
  void exec_load(uint8_t reg, uint8_t base);
  void exec_store(uint8_t base, uint8_t reg);
};

} // namespace srrarch

#endif // SIMULATOR_H