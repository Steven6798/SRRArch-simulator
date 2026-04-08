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
#include <memory>
#include <unordered_map>
#include <vector>

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
  void set_use_block_cache(bool enable) { use_block_cache = enable; }

private:
  Registers regs;
  std::unique_ptr<ElfLoader> loader;
  Memory memory;
  uint64_t entry_point = 0;
  bool running = false;
  bool use_block_cache = false;
  uint64_t instruction_count = 0;
  uint64_t max_instructions = 10000;

  // Basic Block Cache structures
  struct BasicBlock {
    uint64_t start_pc;
    uint64_t end_pc;
    std::vector<Instruction> instructions;
    uint64_t last_executed;
  };

  std::unordered_map<uint64_t, std::unique_ptr<BasicBlock>> block_cache;
  size_t cache_hits = 0;
  size_t cache_misses = 0;

  // Cache management
  BasicBlock *get_basic_block(uint64_t pc);
  void cache_basic_block(uint64_t pc);
  void evict_lru_block();

  // Execution modes
  void run_interpreter();
  void run_block_cache();
  void execute_basic_block(const BasicBlock &block);

  // Fetch and execute
  __attribute__((always_inline)) inline uint64_t fetch();
  void execute(const Instruction &inst);

  // Instruction implementations
  __attribute__((always_inline)) inline void exec_nop();
  __attribute__((always_inline)) inline void exec_return();
  __attribute__((always_inline)) inline void
  exec_genint(const DecodedGenInt &dec);
  __attribute__((always_inline)) inline void exec_mov(const DecodedMov &dec);
  __attribute__((always_inline)) inline void
  exec_select(const DecodedSelect &dec);

  // Arithmetic register-register
  __attribute__((always_inline)) inline void exec_add(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_sub(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_mul(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_sdiv(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_udiv(const DecodedR &dec);

  // Arithmetic register-immediate
  __attribute__((always_inline)) inline void exec_addi(const DecodedRI &dec);
  __attribute__((always_inline)) inline void exec_subi(const DecodedRI &dec);

  // Logical register-register
  __attribute__((always_inline)) inline void exec_and(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_or(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_xor(const DecodedR &dec);

  // Logical register-immediate
  __attribute__((always_inline)) inline void exec_andi(const DecodedRI &dec);
  __attribute__((always_inline)) inline void exec_ori(const DecodedRI &dec);
  __attribute__((always_inline)) inline void exec_xori(const DecodedRI &dec);

  // Shifts register-register
  __attribute__((always_inline)) inline void exec_shl(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_sra(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_srl(const DecodedR &dec);

  // Shifts register-immediate
  __attribute__((always_inline)) inline void exec_shli(const DecodedRI &dec);
  __attribute__((always_inline)) inline void exec_srai(const DecodedRI &dec);
  __attribute__((always_inline)) inline void exec_srli(const DecodedRI &dec);

  // Comparisons (set dest to 1 or 0)
  __attribute__((always_inline)) inline void exec_cmpeq(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_cmpne(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_cmplt(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_cmpgt(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_cmpult(const DecodedR &dec);
  __attribute__((always_inline)) inline void exec_cmpugt(const DecodedR &dec);

  // Memory operations
  __attribute__((always_inline)) inline void exec_storeb(const DecodedMem &dec);
  __attribute__((always_inline)) inline void exec_storeh(const DecodedMem &dec);
  __attribute__((always_inline)) inline void exec_storew(const DecodedMem &dec);
  __attribute__((always_inline)) inline void exec_store(const DecodedMem &dec);

  __attribute__((always_inline)) inline void exec_loadbz(const DecodedMem &dec);
  __attribute__((always_inline)) inline void exec_loadbs(const DecodedMem &dec);
  __attribute__((always_inline)) inline void exec_loadhz(const DecodedMem &dec);
  __attribute__((always_inline)) inline void exec_loadhs(const DecodedMem &dec);
  __attribute__((always_inline)) inline void exec_loadwz(const DecodedMem &dec);
  __attribute__((always_inline)) inline void exec_loadws(const DecodedMem &dec);
  __attribute__((always_inline)) inline void exec_load(const DecodedMem &dec);

  __attribute__((always_inline)) inline void exec_call(const DecodedCall &dec);
  __attribute__((always_inline)) inline void
  exec_callreg(const DecodedCallReg &dec);

  // Special case to handle print until it can be compiled natively.
  __attribute__((always_inline)) inline void exec_printf();

  __attribute__((always_inline)) inline void exec_br(const DecodedBranch &dec);
  __attribute__((always_inline)) inline void
  exec_brcond(const DecodedCondBranch &dec);
};

} // namespace srrarch

#endif // SIMULATOR_H