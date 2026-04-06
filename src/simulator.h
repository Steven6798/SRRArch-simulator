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

  void dump_stack(uint64_t bytes = 128) const;
  void dump_stack_frame() const;

  void set_max_instructions(uint64_t max) { max_instructions = max; }

private:
  Registers regs;
  std::unique_ptr<ElfLoader> loader;
  Memory memory;
  uint64_t entry_point = 0;
  bool running = false;
  uint64_t instruction_count = 0;
  uint64_t max_instructions = 10000;

  // Fetch instruction at current PC
  uint64_t fetch();

  // Execute a single instruction
  void execute(const Instruction &inst);

  // Instruction implementations
  void exec_nop();
  void exec_return();
  void exec_genint(const DecodedGenInt &dec);
  void exec_mov(const DecodedMov &dec);

  // Arithmetic register-register
  void exec_add(const DecodedR &dec);
  void exec_sub(const DecodedR &dec);
  void exec_mul(const DecodedR &dec);
  void exec_sdiv(const DecodedR &dec);
  void exec_udiv(const DecodedR &dec);

  // Arithmetic register-immediate
  void exec_addi(const DecodedRI &dec);
  void exec_subi(const DecodedRI &dec);

  // Logical register-register
  void exec_and(const DecodedR &dec);
  void exec_or(const DecodedR &dec);
  void exec_xor(const DecodedR &dec);

  // Logical register-immediate
  void exec_andi(const DecodedRI &dec);
  void exec_ori(const DecodedRI &dec);
  void exec_xori(const DecodedRI &dec);

  // Shifts register-register
  void exec_shl(const DecodedR &dec);
  void exec_sra(const DecodedR &dec);
  void exec_srl(const DecodedR &dec);

  // Shifts register-immediate
  void exec_shli(const DecodedRI &dec);
  void exec_srai(const DecodedRI &dec);
  void exec_srli(const DecodedRI &dec);

  // Comparisons (set dest to 1 or 0)
  void exec_cmpeq(const DecodedR &dec);
  void exec_cmpne(const DecodedR &dec);
  void exec_cmplt(const DecodedR &dec);
  void exec_cmpgt(const DecodedR &dec);
  void exec_cmpult(const DecodedR &dec);
  void exec_cmpugt(const DecodedR &dec);

  // Memory operations
  void exec_storeb(const DecodedMem &dec);
  void exec_storeh(const DecodedMem &dec);
  void exec_storew(const DecodedMem &dec);
  void exec_store(const DecodedMem &dec);

  void exec_loadbz(const DecodedMem &dec);
  void exec_loadbs(const DecodedMem &dec);
  void exec_loadhz(const DecodedMem &dec);
  void exec_loadhs(const DecodedMem &dec);
  void exec_loadwz(const DecodedMem &dec);
  void exec_loadws(const DecodedMem &dec);
  void exec_load(const DecodedMem &dec);

  void exec_call(const DecodedCall &dec);
  void exec_callreg(const DecodedCallReg &dec);

  // Special case to handle print until it can be compiled natively.
  void exec_printf();

  void exec_br(const DecodedBranch &dec);
  void exec_brcond(const DecodedCondBranch &dec);
};

} // namespace srrarch

#endif // SIMULATOR_H