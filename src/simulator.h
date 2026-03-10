#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "registers.h"
#include <cstdint>
#include <map>
#include <vector>

// Forward declaration
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

private:
  Registers regs;
  ElfLoader *loader;                  // Pointer to ELF loader
  std::map<uint64_t, uint8_t> memory; // Simple memory model: address -> byte
  uint64_t entry_point;
  bool running;
  uint64_t instruction_count;

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
  void exec_shla(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_or(uint8_t dest, uint8_t src1, uint8_t src2);
  void exec_mov(uint8_t dest, uint8_t src);
};

#endif // SIMULATOR_H