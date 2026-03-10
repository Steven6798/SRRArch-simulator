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

#include "registers.h"
#include <cstdint>
#include <map>
#include <memory>
#include <vector>

class ElfLoader;

class Simulator {
public:
  Simulator();
  ~Simulator();

  // Load and prepare for execution
  bool load_elf(const char *filename);

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

  // Delete copy constructor and assignment operator (unique_ptr can't be
  // copied)
  Simulator(const Simulator &) = delete;
  Simulator &operator=(const Simulator &) = delete;

  // Allow move operations
  Simulator(Simulator &&) = default;
  Simulator &operator=(Simulator &&) = default;

private:
  Registers regs;
  std::unique_ptr<ElfLoader> loader;
  std::map<uint64_t, uint8_t> memory;
  uint64_t entry_point = 0;
  bool running = false;
  uint64_t instruction_count = 0;

  // Memory access
  uint64_t read_mem(uint64_t addr, size_t size);
  void write_mem(uint64_t addr, uint64_t value, size_t size);

  // Fetch instruction at current PC
  uint64_t fetch();

  // Execute a single instruction
  void execute(uint64_t instruction);

  // Instruction implementations
  void exec_nop();
  void exec_return(uint8_t reg);
  void exec_genint(uint8_t reg, uint32_t imm);
  void exec_shl(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_or(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_mov(uint8_t dest, uint8_t src);
};

#endif // SIMULATOR_H