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
#include "elf_loader.h"
#include "logger.h"
#include <cassert>
#include <iomanip>

namespace srrarch {

Simulator::Simulator() = default;
Simulator::~Simulator() = default;

LoadResult Simulator::load_elf(const char *filename) {
  LOG_INFO("Loading ELF into simulator: %s", filename);

  // Create and use ElfLoader to parse the ELF
  loader = std::make_unique<ElfLoader>();

  LoadResult result = loader->load(filename);
  if (result != LoadResult::SUCCESS) {
    LOG_ERROR("Failed to load ELF file: %s - %s", filename,
              load_result_to_string(result));
    loader.reset();
    return result;
  }

  // Get entry point
  entry_point = reinterpret_cast<uint64_t>(loader->get_entry_point());
  regs.set_pc(entry_point);
  LOG_INFO("Entry point set to 0x%lx", entry_point);

  // Load executable sections into memory
  const auto &exec_sections = loader->get_executable_sections();
  size_t total_bytes = 0;

  for (const auto &sec : exec_sections) {
    LOG_INFO("Loading executable section %s at 0x%lx (size: 0x%lx bytes)",
             sec.name.c_str(), sec.addr, sec.size);
    memory.load_segment(sec.addr, sec.data, sec.size);
    total_bytes += sec.size;
  }

  // Load data sections into memory
  const auto &data_sections = loader->get_data_sections();
  for (const auto &sec : data_sections) {
    LOG_INFO("Loading data section %s at 0x%lx (size: 0x%lx bytes)",
             sec.name.c_str(), sec.addr, sec.size);

    // Debug: show the first few bytes of .data
    if (sec.name == ".data") {
      LOG_INFO("  .data content: %02x %02x %02x %02x %02x %02x %02x %02x",
               sec.data[0], sec.data[1], sec.data[2], sec.data[3], sec.data[4],
               sec.data[5], sec.data[6], sec.data[7]);
    }

    memory.load_segment(sec.addr, sec.data, sec.size);
    total_bytes += sec.size;
  }

  LOG_INFO("Loaded %zu bytes into memory", total_bytes);

  // Initialize stack pointer
  regs.set_sp(0x80000000);
  LOG_DBG("Stack pointer initialized to 0x%lx", regs.get_sp());

  return LoadResult::SUCCESS;
}

uint64_t Simulator::fetch() {
  uint64_t pc = regs.get_pc();

  // Use Memory class to read instruction
  uint64_t instruction = memory.read_qword(pc);
  LOG_DBG("Fetched instruction at 0x%lx: 0x%016lx", pc, instruction);

  // Advance PC
  regs.set_pc(pc + 8);
  return instruction;
}

void Simulator::execute(const Instruction &inst) {
  LOG_INFO("[%6lu] PC=0x%lx", instruction_count, regs.get_pc() - 8);

  // Decode and print using Instruction class
  inst.print_bytes();
  inst.print();

  switch (inst.opcode()) {
  case Opcode::NOP:
    exec_nop();
    break;

  case Opcode::RETURN:
    exec_return();
    break;

  case Opcode::GENINT:
    exec_genint(inst.genint_reg(), inst.genint_imm());
    break;

  case Opcode::MOV:
    exec_mov(inst.mov_dest(), inst.mov_src());
    break;

  // Arithmetic
  case Opcode::ADD:
    exec_add(inst.arith_dest(), inst.arith_src1(), inst.arith_src2());
    break;
  case Opcode::SUB:
    exec_sub(inst.arith_dest(), inst.arith_src1(), inst.arith_src2());
    break;
  case Opcode::MUL:
    exec_mul(inst.arith_dest(), inst.arith_src1(), inst.arith_src2());
    break;
  case Opcode::SDIV:
    exec_sdiv(inst.arith_dest(), inst.arith_src1(), inst.arith_src2());
    break;
  case Opcode::UDIV:
    exec_udiv(inst.arith_dest(), inst.arith_src1(), inst.arith_src2());
    break;

  // Logical
  case Opcode::AND:
    exec_and(inst.arith_dest(), inst.arith_src1(), inst.arith_src2());
    break;
  case Opcode::OR:
    exec_or(inst.arith_dest(), inst.arith_src1(), inst.arith_src2());
    break;
  case Opcode::XOR:
    exec_xor(inst.arith_dest(), inst.arith_src1(), inst.arith_src2());
    break;

  // Shifts
  case Opcode::SHL:
    exec_shl(inst.shift_dest(), inst.shift_src(), inst.shift_amount());
    break;
  case Opcode::SRA:
    exec_sra(inst.shift_dest(), inst.shift_src(), inst.shift_amount());
    break;
  case Opcode::SRL:
    exec_srl(inst.shift_dest(), inst.shift_src(), inst.shift_amount());
    break;

  // Comparisons
  case Opcode::CMPEQ:
    exec_cmpeq(inst.cmp_dest(), inst.cmp_src1(), inst.cmp_src2());
    break;
  case Opcode::CMPNE:
    exec_cmpne(inst.cmp_dest(), inst.cmp_src1(), inst.cmp_src2());
    break;
  case Opcode::CMPLT:
    exec_cmplt(inst.cmp_dest(), inst.cmp_src1(), inst.cmp_src2());
    break;
  case Opcode::CMPGT:
    exec_cmpgt(inst.cmp_dest(), inst.cmp_src1(), inst.cmp_src2());
    break;

    // Memory
  case Opcode::LOAD:
    exec_load(inst.load_reg(), inst.load_base());
    break;
  case Opcode::STORE:
    exec_store(inst.store_base(), inst.store_reg());
    break;

  case Opcode::CALL:
    exec_call(inst.call_target());
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
  uint64_t raw_inst = fetch();
  if (running) {
    Instruction inst(raw_inst);
    execute(inst);
  }
}

void Simulator::run() {
  running = true;
  instruction_count = 0;

  LOG_INFO("=== Starting simulation ===");
  LOG_INFO("Entry point: 0x%lx", entry_point);
  LOG_DBG("Initial register state:");
  regs.dump();

  while (running) {
    step();
    if (instruction_count > 10000) {
      LOG_WARN("Reached max instruction count (10000)");
      running = false;
    }
  }

  LOG_INFO("=== Simulation finished ===");
  LOG_INFO("Executed %lu instructions", instruction_count);
  LOG_DBG("Final register state:");
  regs.dump();
}

// Instruction implementations
void Simulator::exec_nop() { LOG_DBG("  -> NOP"); }

void Simulator::exec_return() {
  uint64_t return_addr = regs.read(RA_REG);  // R4
  uint64_t return_value = regs.read(RV_REG); // R3

  if (return_addr == 0) {
    // Returning from _start (or any entry with R4=0) - program done
    regs.set_retval(return_value);
    LOG_INFO("  -> RETURN: program finished with value 0x%lx", return_value);
    running = false;
  } else {
    // Normal function return
    regs.set_pc(return_addr);
    LOG_INFO("  -> RETURN: to 0x%lx (from R4), value 0x%lx in R3", return_addr,
             return_value);
  }
}

void Simulator::exec_genint(uint8_t reg, uint32_t imm) {
  LOG_INFO("  -> GENINT: R%u = 0x%x", reg, imm);
  regs.write(reg, static_cast<uint64_t>(imm));
}

void Simulator::exec_mov(uint8_t dest, uint8_t src) {
  uint64_t val = regs.read(src);
  LOG_INFO("  -> MOV: R%u = R%u (0x%lx)", dest, src, val);
  regs.write(dest, val);
}

// Arithmetic
void Simulator::exec_add(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t a = regs.read(src1);
  uint64_t b = regs.read(src2);
  uint64_t result = a + b;
  LOG_INFO("  -> ADD: R%u = R%u + R%u (0x%lx + 0x%lx = 0x%lx)", dest, src1,
           src2, a, b, result);
  regs.write(dest, result);
}

void Simulator::exec_sub(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t a = regs.read(src1);
  uint64_t b = regs.read(src2);
  uint64_t result = a - b;
  LOG_INFO("  -> SUB: R%u = R%u - R%u (0x%lx - 0x%lx = 0x%lx)", dest, src1,
           src2, a, b, result);
  regs.write(dest, result);
}

void Simulator::exec_mul(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t a = regs.read(src1);
  uint64_t b = regs.read(src2);
  uint64_t result = a * b;
  LOG_INFO("  -> MUL: R%u = R%u * R%u (0x%lx * 0x%lx = 0x%lx)", dest, src1,
           src2, a, b, result);
  regs.write(dest, result);
}

void Simulator::exec_sdiv(uint8_t dest, uint8_t src1, uint8_t src2) {
  int64_t a = static_cast<int64_t>(regs.read(src1));
  int64_t b = static_cast<int64_t>(regs.read(src2));
  if (b == 0) {
    LOG_ERROR("  -> SDIV: division by zero!");
    running = false;
    return;
  }
  int64_t result = a / b;
  LOG_INFO("  -> SDIV: R%u = R%u / R%u (%ld / %ld = %ld)", dest, src1, src2, a,
           b, result);
  regs.write(dest, static_cast<uint64_t>(result));
}

void Simulator::exec_udiv(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t a = regs.read(src1);
  uint64_t b = regs.read(src2);
  if (b == 0) {
    LOG_ERROR("  -> UDIV: division by zero!");
    running = false;
    return;
  }
  uint64_t result = a / b;
  LOG_INFO("  -> UDIV: R%u = R%u / R%u (0x%lx / 0x%lx = 0x%lx)", dest, src1,
           src2, a, b, result);
  regs.write(dest, result);
}

// Logical
void Simulator::exec_and(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t a = regs.read(src1);
  uint64_t b = regs.read(src2);
  uint64_t result = a & b;
  LOG_INFO("  -> AND: R%u = R%u & R%u (0x%lx & 0x%lx = 0x%lx)", dest, src1,
           src2, a, b, result);
  regs.write(dest, result);
}

void Simulator::exec_or(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t a = regs.read(src1);
  uint64_t b = regs.read(src2);
  uint64_t result = a | b;
  LOG_INFO("  -> OR: R%u = R%u | R%u (0x%lx | 0x%lx = 0x%lx)", dest, src1, src2,
           a, b, result);
  regs.write(dest, result);
}

void Simulator::exec_xor(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t a = regs.read(src1);
  uint64_t b = regs.read(src2);
  uint64_t result = a ^ b;
  LOG_INFO("  -> XOR: R%u = R%u ^ R%u (0x%lx ^ 0x%lx = 0x%lx)", dest, src1,
           src2, a, b, result);
  regs.write(dest, result);
}

// Shifts
void Simulator::exec_shl(uint8_t dest, uint8_t src, uint8_t amount) {
  uint64_t val = regs.read(src);
  uint64_t shift = regs.read(amount) & 0x3F; // Only low 6 bits used
  uint64_t result = val << shift;
  LOG_INFO("  -> SHL: R%u = R%u << R%u (0x%lx << %lu = 0x%lx)", dest, src,
           amount, val, shift, result);
  regs.write(dest, result);
}

void Simulator::exec_sra(uint8_t dest, uint8_t src, uint8_t amount) {
  int64_t val = static_cast<int64_t>(regs.read(src));
  uint64_t shift = regs.read(amount) & 0x3F;
  int64_t result = val >> shift; // Arithmetic shift
  LOG_INFO("  -> SRA: R%u = R%u >> R%u (%ld >> %lu = %ld)", dest, src, amount,
           val, shift, result);
  regs.write(dest, static_cast<uint64_t>(result));
}

void Simulator::exec_srl(uint8_t dest, uint8_t src, uint8_t amount) {
  uint64_t val = regs.read(src);
  uint64_t shift = regs.read(amount) & 0x3F;
  uint64_t result = val >> shift; // Logical shift
  LOG_INFO("  -> SRL: R%u = R%u >> R%u (0x%lx >> %lu = 0x%lx)", dest, src,
           amount, val, shift, result);
  regs.write(dest, result);
}

// Comparisons
void Simulator::exec_cmpeq(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t a = regs.read(src1);
  uint64_t b = regs.read(src2);
  uint64_t result = (a == b) ? 1 : 0;
  LOG_INFO("  -> CMPEQ: R%u = (R%u == R%u) ? 1 : 0 (0x%lx == 0x%lx = %lu)",
           dest, src1, src2, a, b, result);
  regs.write(dest, result);
}

void Simulator::exec_cmpne(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t a = regs.read(src1);
  uint64_t b = regs.read(src2);
  uint64_t result = (a != b) ? 1 : 0;
  LOG_INFO("  -> CMPNE: R%u = (R%u != R%u) ? 1 : 0 (0x%lx != 0x%lx = %lu)",
           dest, src1, src2, a, b, result);
  regs.write(dest, result);
}

void Simulator::exec_cmplt(uint8_t dest, uint8_t src1, uint8_t src2) {
  int64_t a = static_cast<int64_t>(regs.read(src1));
  int64_t b = static_cast<int64_t>(regs.read(src2));
  uint64_t result = (a < b) ? 1 : 0;
  LOG_INFO("  -> CMPLT: R%u = (R%u < R%u) ? 1 : 0 (%ld < %ld = %lu)", dest,
           src1, src2, a, b, result);
  regs.write(dest, result);
}

void Simulator::exec_cmpgt(uint8_t dest, uint8_t src1, uint8_t src2) {
  int64_t a = static_cast<int64_t>(regs.read(src1));
  int64_t b = static_cast<int64_t>(regs.read(src2));
  uint64_t result = (a > b) ? 1 : 0;
  LOG_INFO("  -> CMPGT: R%u = (R%u > R%u) ? 1 : 0 (%ld > %ld = %lu)", dest,
           src1, src2, a, b, result);
  regs.write(dest, result);
}

// Memory operations
void Simulator::exec_load(uint8_t reg, uint8_t base) {
  uint64_t addr = regs.read(base);
  uint64_t value = memory.read_qword(addr);
  LOG_INFO("  -> LOAD: R%u = [R%u] (0x%lx = 0x%lx)", reg, base, addr, value);
  regs.write(reg, value);
}

void Simulator::exec_store(uint8_t base, uint8_t reg) {
  uint64_t addr = regs.read(base);
  uint64_t value = regs.read(reg);
  LOG_INFO("  -> STORE: [R%u] = R%u (0x%lx = 0x%lx)", base, reg, addr, value);
  memory.write_qword(addr, value);
}

void Simulator::exec_call(uint8_t target_reg) {
  uint64_t target = regs.read(target_reg);
  uint64_t return_addr = regs.get_pc(); // PC already advanced by fetch()

  // Save return address to link register (R4)
  regs.write(RA_REG, return_addr);

  // Jump to target
  regs.set_pc(target);

  LOG_INFO("  -> CALL R%u (0x%lx), return addr 0x%lx saved to R4", target_reg,
           target, return_addr);
}

} // namespace srrarch