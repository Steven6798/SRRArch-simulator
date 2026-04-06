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
#include "memory.h"
#include <iostream>

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
    if (sec.is_bss) {
      // For BSS, zero-initialize the memory
      LOG_INFO("Zero-initializing BSS section %s at 0x%lx (size: 0x%lx bytes)",
               sec.name.c_str(), sec.addr, sec.size);
      std::vector<uint8_t> zeros(sec.size, 0);
      memory.load_segment(sec.addr, zeros.data(), sec.size);
    } else {
      LOG_INFO("Loading data section %s at 0x%lx (size: 0x%lx bytes)",
               sec.name.c_str(), sec.addr, sec.size);
      memory.load_segment(sec.addr, sec.data, sec.size);
    }
  }

  LOG_INFO("Loaded %zu bytes into memory", total_bytes);

  // Initialize stack and frame pointers
  regs.set_sp(0x80000000);
  regs.set_fp(0x80000000);
  LOG_DBG("Stack pointer initialized to 0x%lx", regs.get_sp());
  LOG_DBG("Frame pointer initialized to 0x%lx", regs.get_fp());

  return LoadResult::SUCCESS;
}

uint64_t Simulator::fetch() {
  const uint64_t pc = regs.get_pc();

  // Validate PC is within mapped executable section
  if (!memory.is_mapped(pc)) {
    LOG_ERROR("Invalid PC: 0x%lx not mapped in memory!", pc);
    running = false;
    return 0;
  }

  // Optional: Check alignment (if instructions are 8-byte aligned)
  if (pc % 8 != 0) {
    LOG_ERROR("Invalid PC: 0x%lx is not 8-byte aligned!", pc);
    running = false;
    return 0;
  }

  // Use Memory class to read instruction
  const uint64_t instruction = memory.read_qword(pc);
  LOG_DBG("Fetched instruction at 0x%lx: 0x%016lx", pc, instruction);

  // Advance PC
  regs.set_pc(pc + 8);
  return instruction;
}

void Simulator::execute(const Instruction &inst) {
  const Opcode op = inst.opcode();
  // Validate opcode before execution
  if (static_cast<uint8_t>(op) >= static_cast<uint8_t>(Opcode::COUNT)) {
    LOG_ERROR("Invalid opcode 0x%02x at PC=0x%lx", op, regs.get_pc() - 8);
    LOG_ERROR("  Raw instruction: 0x%016lx", inst.raw_value());
    LOG_ERROR("  Halting simulation");
    running = false;
    return;
  }

  LOG_INFO("[%6lu] PC=0x%lx", instruction_count, regs.get_pc() - 8);

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_NONE
  inst.print_bytes();
  inst.print();
#endif

  // Dispatch using pre-decoded data
  switch (op) {
  case Opcode::NOP:
    exec_nop();
    break;

  case Opcode::RETURN:
    exec_return();
    break;

  case Opcode::GENINT:
    exec_genint(inst.as_genint());
    break;

  case Opcode::MOV:
    exec_mov(inst.as_mov());
    break;

  // Arithmetic register-register
  case Opcode::ADD:
    exec_add(inst.as_r());
    break;
  case Opcode::SUB:
    exec_sub(inst.as_r());
    break;
  case Opcode::MUL:
    exec_mul(inst.as_r());
    break;
  case Opcode::SDIV:
    exec_sdiv(inst.as_r());
    break;
  case Opcode::UDIV:
    exec_udiv(inst.as_r());
    break;

  // Arithmetic register-immediate
  case Opcode::ADDI:
    exec_addi(inst.as_ri());
    break;
  case Opcode::SUBI:
    exec_subi(inst.as_ri());
    break;

  // Logical register-register
  case Opcode::AND:
    exec_and(inst.as_r());
    break;
  case Opcode::OR:
    exec_or(inst.as_r());
    break;
  case Opcode::XOR:
    exec_xor(inst.as_r());
    break;

  // Logical register-immediate
  case Opcode::ANDI:
    exec_andi(inst.as_ri());
    break;
  case Opcode::ORI:
    exec_ori(inst.as_ri());
    break;
  case Opcode::XORI:
    exec_xori(inst.as_ri());
    break;

  // Shifts register-register
  case Opcode::SHL:
    exec_shl(inst.as_r());
    break;
  case Opcode::SRA:
    exec_sra(inst.as_r());
    break;
  case Opcode::SRL:
    exec_srl(inst.as_r());
    break;

  // Shifts register-immediate
  case Opcode::SHLI:
    exec_shli(inst.as_ri());
    break;
  case Opcode::SRAI:
    exec_srai(inst.as_ri());
    break;
  case Opcode::SRLI:
    exec_srli(inst.as_ri());
    break;

  // Comparisons
  case Opcode::CMPEQ:
    exec_cmpeq(inst.as_r());
    break;
  case Opcode::CMPNE:
    exec_cmpne(inst.as_r());
    break;
  case Opcode::CMPLT:
    exec_cmplt(inst.as_r());
    break;
  case Opcode::CMPGT:
    exec_cmpgt(inst.as_r());
    break;
  case Opcode::CMPULT:
    exec_cmpult(inst.as_r());
    break;
  case Opcode::CMPUGT:
    exec_cmpugt(inst.as_r());
    break;

  // Memory
  case Opcode::STOREB:
    exec_storeb(inst.as_mem());
    break;
  case Opcode::STOREH:
    exec_storeh(inst.as_mem());
    break;
  case Opcode::STOREW:
    exec_storew(inst.as_mem());
    break;
  case Opcode::STORE:
    exec_store(inst.as_mem());
    break;

  case Opcode::LOADBZ:
    exec_loadbz(inst.as_mem());
    break;
  case Opcode::LOADBS:
    exec_loadbs(inst.as_mem());
    break;
  case Opcode::LOADHZ:
    exec_loadhz(inst.as_mem());
    break;
  case Opcode::LOADHS:
    exec_loadhs(inst.as_mem());
    break;
  case Opcode::LOADWZ:
    exec_loadwz(inst.as_mem());
    break;
  case Opcode::LOADWS:
    exec_loadws(inst.as_mem());
    break;
  case Opcode::LOAD:
    exec_load(inst.as_mem());
    break;

  case Opcode::CALL:
    exec_call(inst.as_call());
    break;
  case Opcode::CALLREG:
    exec_callreg(inst.as_callreg());
    break;

  case Opcode::BR:
    exec_br(inst.as_branch());
    break;
  case Opcode::BRCOND:
    exec_brcond(inst.as_cond_branch());
    break;

  default:
    LOG_ERROR("Unknown opcode at PC=0x%lx", regs.get_pc() - 8);
    running = false;
    break;
  }

  instruction_count++;
}

void Simulator::run() {
  running = true;
  instruction_count = 0;
  LOG_INFO("=== Starting simulation ===");
  LOG_INFO("Entry point: 0x%lx", entry_point);
  // LOG_DBG("Initial register state:");
  //   regs.dump();

  while (running) {
    uint64_t raw_inst = fetch();
    if (!running)
      break;

    Instruction inst(raw_inst);
    execute(inst);
    if (instruction_count >= max_instructions) {
      LOG_WARN("Reached max instruction count (%lu)", max_instructions);
      running = false;
    }
  }

  LOG_INFO("=== Simulation finished ===");
  LOG_INFO("Executed %lu instructions", instruction_count);
  // LOG_DBG("Final register state:");
  //   regs.dump();
}

// Instruction implementations
void Simulator::exec_nop() { LOG_DBG("  -> NOP"); }

void Simulator::exec_return() {
  const uint64_t return_addr = regs.get_ra();
  const uint64_t return_value = regs.get_retval();

  if (return_addr == 0) {
    // Returning from _start (or any entry with RA=0) - program done
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

void Simulator::exec_genint(const DecodedGenInt &dec) {
  LOG_INFO("  -> GENINT: R%u = 0x%x", dec.reg, dec.imm);
  regs.write(dec.reg, static_cast<uint64_t>(dec.imm));
}

void Simulator::exec_mov(const DecodedMov &dec) {
  const uint64_t val = regs.read(dec.src);
  LOG_INFO("  -> MOV: R%u = R%u (0x%lx)", dec.dest, dec.src, val);
  regs.write(dec.dest, val);
}

// Arithmetic register-register
void Simulator::exec_add(const DecodedR &dec) {
  const uint64_t a = regs.read(dec.src1);
  const uint64_t b = regs.read(dec.src2);
  const uint64_t result = a + b;
  LOG_INFO("  -> ADD: R%u = R%u + R%u (0x%lx + 0x%lx = 0x%lx)", dec.dest,
           dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_sub(const DecodedR &dec) {
  const uint64_t a = regs.read(dec.src1);
  const uint64_t b = regs.read(dec.src2);
  const uint64_t result = a - b;
  LOG_INFO("  -> SUB: R%u = R%u - R%u (0x%lx - 0x%lx = 0x%lx)", dec.dest,
           dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_mul(const DecodedR &dec) {
  const uint64_t a = regs.read(dec.src1);
  const uint64_t b = regs.read(dec.src2);
  const uint64_t result = a * b;
  LOG_INFO("  -> MUL: R%u = R%u * R%u (0x%lx * 0x%lx = 0x%lx)", dec.dest,
           dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_sdiv(const DecodedR &dec) {
  const int64_t a = static_cast<int64_t>(regs.read(dec.src1));
  const int64_t b = static_cast<int64_t>(regs.read(dec.src2));
  if (b == 0) {
    LOG_ERROR("  -> SDIV: division by zero!");
    running = false;
    return;
  }
  const int64_t result = a / b;
  LOG_INFO("  -> SDIV: R%u = R%u / R%u (%ld / %ld = %ld)", dec.dest, dec.src1,
           dec.src2, a, b, result);
  regs.write(dec.dest, static_cast<uint64_t>(result));
}

void Simulator::exec_udiv(const DecodedR &dec) {
  const uint64_t a = regs.read(dec.src1);
  const uint64_t b = regs.read(dec.src2);
  if (b == 0) {
    LOG_ERROR("  -> UDIV: division by zero!");
    running = false;
    return;
  }
  const uint64_t result = a / b;
  LOG_INFO("  -> UDIV: R%u = R%u / R%u (0x%lx / 0x%lx = 0x%lx)", dec.dest,
           dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

// Arithmetic register-immediate
void Simulator::exec_addi(const DecodedRI &dec) {
  const uint64_t a = regs.read(dec.src);
  const uint64_t result = a + static_cast<uint64_t>(dec.imm);
  LOG_INFO("  -> ADDI: R%u = R%u + %d (0x%lx + %d = 0x%lx)", dec.dest, dec.src,
           dec.imm, a, dec.imm, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_subi(const DecodedRI &dec) {
  const uint64_t a = regs.read(dec.src);
  const uint64_t result = a - static_cast<uint64_t>(dec.imm);
  LOG_INFO("  -> SUBI: R%u = R%u - %d (0x%lx - %d = 0x%lx)", dec.dest, dec.src,
           dec.imm, a, dec.imm, result);
  regs.write(dec.dest, result);
}

// Logical register-register
void Simulator::exec_and(const DecodedR &dec) {
  const uint64_t a = regs.read(dec.src1);
  const uint64_t b = regs.read(dec.src2);
  const uint64_t result = a & b;
  LOG_INFO("  -> AND: R%u = R%u & R%u (0x%lx & 0x%lx = 0x%lx)", dec.dest,
           dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_or(const DecodedR &dec) {
  const uint64_t a = regs.read(dec.src1);
  const uint64_t b = regs.read(dec.src2);
  const uint64_t result = a | b;
  LOG_INFO("  -> OR: R%u = R%u | R%u (0x%lx | 0x%lx = 0x%lx)", dec.dest,
           dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_xor(const DecodedR &dec) {
  const uint64_t a = regs.read(dec.src1);
  const uint64_t b = regs.read(dec.src2);
  const uint64_t result = a ^ b;
  LOG_INFO("  -> XOR: R%u = R%u ^ R%u (0x%lx ^ 0x%lx = 0x%lx)", dec.dest,
           dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

// Logical register-immediate
void Simulator::exec_andi(const DecodedRI &dec) {
  const uint64_t a = regs.read(dec.src);
  const uint64_t result = a & static_cast<uint64_t>(dec.imm);
  LOG_INFO("  -> ANDI: R%u = R%u & %d (0x%lx & %d = 0x%lx)", dec.dest, dec.src,
           dec.imm, a, dec.imm, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_ori(const DecodedRI &dec) {
  const uint64_t a = regs.read(dec.src);
  const uint64_t result = a | static_cast<uint64_t>(dec.imm);
  LOG_INFO("  -> ORI: R%u = R%u | %d (0x%lx | %d = 0x%lx)", dec.dest, dec.src,
           dec.imm, a, dec.imm, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_xori(const DecodedRI &dec) {
  const uint64_t a = regs.read(dec.src);
  const uint64_t result = a ^ static_cast<uint64_t>(dec.imm);
  LOG_INFO("  -> XORI: R%u = R%u ^ %d (0x%lx ^ %d = 0x%lx)", dec.dest, dec.src,
           dec.imm, a, dec.imm, result);
  regs.write(dec.dest, result);
}

// Shifts register-register
void Simulator::exec_shl(const DecodedR &dec) {
  const uint64_t val = regs.read(dec.src1);
  const uint64_t shift = regs.read(dec.src2) & 0x3F;
  const uint64_t result = val << shift;
  LOG_INFO("  -> SHL: R%u = R%u << R%u (0x%lx << %lu = 0x%lx)", dec.dest,
           dec.src1, dec.src2, val, shift, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_sra(const DecodedR &dec) {
  const int64_t val = static_cast<int64_t>(regs.read(dec.src1));
  const uint64_t shift = regs.read(dec.src2) & 0x3F;
  const int64_t result = val >> shift;
  LOG_INFO("  -> SRA: R%u = R%u >> R%u (%ld >> %lu = %ld)", dec.dest, dec.src1,
           dec.src2, val, shift, result);
  regs.write(dec.dest, static_cast<uint64_t>(result));
}

void Simulator::exec_srl(const DecodedR &dec) {
  const uint64_t val = regs.read(dec.src1);
  const uint64_t shift = regs.read(dec.src2) & 0x3F;
  const uint64_t result = val >> shift;
  LOG_INFO("  -> SRL: R%u = R%u >> R%u (0x%lx >> %lu = 0x%lx)", dec.dest,
           dec.src1, dec.src2, val, shift, result);
  regs.write(dec.dest, result);
}

// Shifts register-immediate
void Simulator::exec_shli(const DecodedRI &dec) {
  const uint64_t a = regs.read(dec.src);
  const uint64_t result = a << dec.imm;
  LOG_INFO("  -> SHLI: R%u = R%u << %u (0x%lx << %u = 0x%lx)", dec.dest,
           dec.src, dec.imm, a, dec.imm, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_srai(const DecodedRI &dec) {
  const int64_t a = static_cast<int64_t>(regs.read(dec.src));
  const int64_t result = a >> dec.imm;
  LOG_INFO("  -> SRAI: R%u = R%u >> %u (%ld >> %u = %ld)", dec.dest, dec.src,
           dec.imm, a, dec.imm, result);
  regs.write(dec.dest, static_cast<uint64_t>(result));
}

void Simulator::exec_srli(const DecodedRI &dec) {
  const uint64_t a = regs.read(dec.src);
  const uint64_t result = a >> dec.imm;
  LOG_INFO("  -> SRLI: R%u = R%u >> %u (0x%lx >> %u = 0x%lx)", dec.dest,
           dec.src, dec.imm, a, dec.imm, result);
  regs.write(dec.dest, result);
}

// Comparisons
void Simulator::exec_cmpeq(const DecodedR &dec) {
  const uint64_t a = regs.read(dec.src1);
  const uint64_t b = regs.read(dec.src2);
  const uint64_t result = (a == b) ? 1 : 0;
  LOG_INFO("  -> CMPEQ: R%u = (R%u == R%u) ? 1 : 0 (0x%lx == 0x%lx = %lu)",
           dec.dest, dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_cmpne(const DecodedR &dec) {
  const uint64_t a = regs.read(dec.src1);
  const uint64_t b = regs.read(dec.src2);
  const uint64_t result = (a != b) ? 1 : 0;
  LOG_INFO("  -> CMPNE: R%u = (R%u != R%u) ? 1 : 0 (0x%lx != 0x%lx = %lu)",
           dec.dest, dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_cmplt(const DecodedR &dec) {
  const int64_t a = static_cast<int64_t>(regs.read(dec.src1));
  const int64_t b = static_cast<int64_t>(regs.read(dec.src2));
  const uint64_t result = (a < b) ? 1 : 0;
  LOG_INFO("  -> CMPLT: R%u = (R%u < R%u) ? 1 : 0 (%ld < %ld = %lu)", dec.dest,
           dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_cmpgt(const DecodedR &dec) {
  const int64_t a = static_cast<int64_t>(regs.read(dec.src1));
  const int64_t b = static_cast<int64_t>(regs.read(dec.src2));
  const uint64_t result = (a > b) ? 1 : 0;
  LOG_INFO("  -> CMPGT: R%u = (R%u > R%u) ? 1 : 0 (%ld > %ld = %lu)", dec.dest,
           dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_cmpult(const DecodedR &dec) {
  const uint64_t a = regs.read(dec.src1);
  const uint64_t b = regs.read(dec.src2);
  const uint64_t result = (a < b) ? 1 : 0;
  LOG_INFO("  -> CMPULT: R%u = (R%u < R%u) ? 1 : 0 (%lu < %lu = %lu)", dec.dest,
           dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

void Simulator::exec_cmpugt(const DecodedR &dec) {
  const uint64_t a = regs.read(dec.src1);
  const uint64_t b = regs.read(dec.src2);
  const uint64_t result = (a > b) ? 1 : 0;
  LOG_INFO("  -> CMPUGT: R%u = (R%u > R%u) ? 1 : 0 (%lu > %lu = %lu)", dec.dest,
           dec.src1, dec.src2, a, b, result);
  regs.write(dec.dest, result);
}

// Memory operations
void Simulator::exec_storeb(const DecodedMem &dec) {
  const uint64_t addr = static_cast<uint64_t>(
      static_cast<int64_t>(regs.read(dec.base)) + dec.offset);
  const uint64_t value = regs.read(dec.reg);
  memory.write_byte(addr, value & 0xFF);
  LOG_INFO("  -> STOREB: [R%u + %d] = R%u (0x%lx = 0x%02llx)", dec.base,
           dec.offset, dec.reg, addr, value & 0xFF);
}

void Simulator::exec_storeh(const DecodedMem &dec) {
  const uint64_t addr = static_cast<uint64_t>(
      static_cast<int64_t>(regs.read(dec.base)) + dec.offset);
  const uint64_t value = regs.read(dec.reg);
  memory.write_word(addr, value & 0xFFFF);
  LOG_INFO("  -> STOREH: [R%u + %d] = R%u (0x%lx = 0x%04llx)", dec.base,
           dec.offset, dec.reg, addr, value & 0xFFFF);
}

void Simulator::exec_storew(const DecodedMem &dec) {
  const uint64_t addr = static_cast<uint64_t>(
      static_cast<int64_t>(regs.read(dec.base)) + dec.offset);
  const uint64_t value = regs.read(dec.reg);
  memory.write_dword(addr, value & 0xFFFFFFFF);
  LOG_INFO("  -> STOREW: [R%u + %d] = R%u (0x%lx = 0x%08llx)", dec.base,
           dec.offset, dec.reg, addr, value & 0xFFFFFFFF);
}

void Simulator::exec_store(const DecodedMem &dec) {
  const uint64_t addr = static_cast<uint64_t>(
      static_cast<int64_t>(regs.read(dec.base)) + dec.offset);
  const uint64_t value = regs.read(dec.reg);
  memory.write_qword(addr, value);
  LOG_INFO("  -> STORE: [R%u + %d] = R%u (0x%lx = 0x%lx)", dec.base, dec.offset,
           dec.reg, addr, value);
}

void Simulator::exec_loadbz(const DecodedMem &dec) {
  const uint64_t addr = static_cast<uint64_t>(
      static_cast<int64_t>(regs.read(dec.base)) + dec.offset);
  const uint8_t value = memory.read_byte(addr);
  regs.write(dec.reg, value);
  LOG_INFO("  -> LOADBZ: R%u = [R%u + %d] (0x%lx = 0x%02llx)", dec.reg,
           dec.base, dec.offset, addr, value);
}

void Simulator::exec_loadbs(const DecodedMem &dec) {
  const uint64_t addr = static_cast<uint64_t>(
      static_cast<int64_t>(regs.read(dec.base)) + dec.offset);
  const int8_t value = static_cast<int8_t>(memory.read_byte(addr));
  regs.write(dec.reg, static_cast<uint64_t>(value));
  LOG_INFO("  -> LOADBS: R%u = [R%u + %d] (0x%lx = 0x%02llx -> 0x%016llx)",
           dec.reg, dec.base, dec.offset, addr, static_cast<uint8_t>(value),
           regs.read(dec.reg));
}

void Simulator::exec_loadhz(const DecodedMem &dec) {
  const uint64_t addr = static_cast<uint64_t>(
      static_cast<int64_t>(regs.read(dec.base)) + dec.offset);
  const uint16_t value = memory.read_word(addr);
  regs.write(dec.reg, value);
  LOG_INFO("  -> LOADHZ: R%u = [R%u + %d] (0x%lx = 0x%04llx)", dec.reg,
           dec.base, dec.offset, addr, value);
}

void Simulator::exec_loadhs(const DecodedMem &dec) {
  const uint64_t addr = static_cast<uint64_t>(
      static_cast<int64_t>(regs.read(dec.base)) + dec.offset);
  const int16_t value = static_cast<int16_t>(memory.read_word(addr));
  regs.write(dec.reg, static_cast<uint64_t>(value));
  LOG_INFO("  -> LOADHS: R%u = [R%u + %d] (0x%lx = 0x%04llx -> 0x%016llx)",
           dec.reg, dec.base, dec.offset, addr, static_cast<uint16_t>(value),
           regs.read(dec.reg));
}

void Simulator::exec_loadwz(const DecodedMem &dec) {
  const uint64_t addr = static_cast<uint64_t>(
      static_cast<int64_t>(regs.read(dec.base)) + dec.offset);
  const uint32_t value = memory.read_dword(addr);
  regs.write(dec.reg, value);
  LOG_INFO("  -> LOADWZ: R%u = [R%u + %d] (0x%lx = 0x%08llx)", dec.reg,
           dec.base, dec.offset, addr, value);
}

void Simulator::exec_loadws(const DecodedMem &dec) {
  const uint64_t addr = static_cast<uint64_t>(
      static_cast<int64_t>(regs.read(dec.base)) + dec.offset);
  const int32_t value = static_cast<int32_t>(memory.read_dword(addr));
  regs.write(dec.reg, static_cast<uint64_t>(value));
  LOG_INFO("  -> LOADWS: R%u = [R%u + %d] (0x%lx = 0x%08llx -> 0x%016llx)",
           dec.reg, dec.base, dec.offset, addr, static_cast<uint32_t>(value),
           regs.read(dec.reg));
}

void Simulator::exec_load(const DecodedMem &dec) {
  const uint64_t addr = static_cast<uint64_t>(
      static_cast<int64_t>(regs.read(dec.base)) + dec.offset);
  const uint64_t value = memory.read_qword(addr);
  regs.write(dec.reg, value);
  LOG_INFO("  -> LOAD: R%u = [R%u + %d] (0x%lx = 0x%lx)", dec.reg, dec.base,
           dec.offset, addr, value);
}

void Simulator::exec_call(const DecodedCall &dec) {
  // dump_stack_frame();
  const uint64_t target = dec.target;
  const uint64_t return_addr = regs.get_pc();

  // Simple check for printf
  if (loader && loader->is_printf_undefined() && target == 0) {
    LOG_INFO("  -> CALL to printf detected");
    exec_printf();
    return;
  }

  // Validate target address
  if (!memory.is_mapped(target)) {
    LOG_ERROR("  -> CALL 0x%lx: target address 0x%lx is not mapped in memory!",
              target, target);
    LOG_ERROR("  -> Halting simulation due to invalid call target");
    running = false;
    return;
  }

  // Save return address to return address register
  regs.set_ra(return_addr);

  // Jump to target
  regs.set_pc(target);

  LOG_INFO("  -> CALL 0x%lx, return addr 0x%lx saved to R4", target,
           return_addr);
}

void Simulator::exec_callreg(const DecodedCallReg &dec) {
  const uint64_t target = regs.read(dec.target_reg);
  const uint64_t return_addr = regs.get_pc();

  // Simple check for printf
  if (loader && loader->is_printf_undefined() && target == 0) {
    LOG_INFO("  -> CALL to printf detected");
    exec_printf();
    return;
  }

  // Validate target address
  if (!memory.is_mapped(target)) {
    LOG_ERROR("  -> CALL R%u: target address 0x%lx is not mapped in memory!",
              dec.target_reg, target);
    LOG_ERROR("  -> Halting simulation due to invalid call target");
    running = false;
    return;
  }

  // Save return address to return address register
  regs.set_ra(return_addr);

  // Jump to target
  regs.set_pc(target);

  LOG_INFO("  -> CALL R%u (0x%lx), return addr 0x%lx saved to R4",
           dec.target_reg, target, return_addr);
}

void Simulator::exec_printf() {
  // All printf calls use the same argument access pattern
  const uint64_t args_base = regs.get_sp();
  if (!memory.is_mapped(args_base)) {
    LOG_ERROR("  -> printf error: args_base 0x%lx not mapped!", args_base);
    running = false;
    return;
  }

  const uint64_t format_addr = memory.read_qword(args_base);
  const uint64_t arg1 = memory.read_qword(args_base + 8);
  const uint64_t arg2 = memory.read_qword(args_base + 16);
  std::string format;

  // Validate format string address
  if (!memory.is_mapped(format_addr)) {
    LOG_ERROR("  -> printf error: format string address 0x%lx not mapped!",
              format_addr);
    running = false;
    return;
  }

  // Read format string from memory
  uint64_t addr = format_addr;
  uint8_t byte;
  while ((byte = memory.read_byte(addr++)) != 0) {
    format += static_cast<char>(byte);
  }

  // Count format specifiers to determine how many arguments to use
  int expected_args = 0;
  for (size_t i = 0; i < format.length(); i++) {
    if (format[i] == '%' && i + 1 < format.length()) {
      char next = format[i + 1];
      if (next != '%') { // %% is escaped percent
        expected_args++;
      }
    }
  }

  // Build output string with proper formatting
  std::string output;
  size_t arg_idx = 0;

  for (size_t i = 0; i < format.length(); i++) {
    if (format[i] == '%' && i + 1 < format.length()) {
      char spec = format[i + 1];
      i++; // Skip the initial '%'

      // Check for length modifiers
      // 0 = default (int), 1 = h (short), 2 = hh (char), 3 = l (long)
      int length_modifier = 0;

      if (spec == 'h') {
        // Check for 'hh' (two h's)
        if (i + 1 < format.length() && format[i + 1] == 'h') {
          length_modifier = 2;
          spec = format[i + 2];
          i += 2; // Skip both 'h' characters
        } else {
          length_modifier = 1;
          spec = format[i + 1];
          i++; // Skip the 'h'
        }
      } else if (spec == 'l') {
        length_modifier = 3;
        spec = format[i + 1];
        i++; // Skip the 'l'
      }

      if (spec == '%') {
        output += '%'; // Escaped percent
        continue;
      }

      // Get the appropriate argument
      uint64_t current_arg = 0;
      if (arg_idx == 0)
        current_arg = arg1;
      else if (arg_idx == 1)
        current_arg = arg2;

      // Format based on specifier and length modifier
      char buffer[32];

      switch (spec) {
      case 'd':
      case 'i': {
        // Signed integer conversion with length modifiers
        switch (length_modifier) {
        case 2: { // hh - char
          signed char val = static_cast<signed char>(current_arg & 0xFF);
          snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(val));
          break;
        }
        case 1: { // h - short
          short val = static_cast<short>(current_arg & 0xFFFF);
          snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(val));
          break;
        }
        default: { // default - int/long
          snprintf(buffer, sizeof(buffer), "%ld",
                   static_cast<long>(current_arg));
          break;
        }
        }
        output += buffer;
        break;
      }
      case 'u': {
        // Unsigned integer conversion with length modifiers
        switch (length_modifier) {
        case 2: { // hh - unsigned char
          unsigned char val = static_cast<unsigned char>(current_arg & 0xFF);
          snprintf(buffer, sizeof(buffer), "%u",
                   static_cast<unsigned int>(val));
          break;
        }
        case 1: { // h - unsigned short
          unsigned short val =
              static_cast<unsigned short>(current_arg & 0xFFFF);
          snprintf(buffer, sizeof(buffer), "%u",
                   static_cast<unsigned int>(val));
          break;
        }
        default: { // default - unsigned long
          snprintf(buffer, sizeof(buffer), "%lu",
                   static_cast<unsigned long>(current_arg));
          break;
        }
        }
        output += buffer;
        break;
      }
      case 'x':
      case 'X': {
        // Hexadecimal conversion with length modifiers
        const char *fmt = (spec == 'x') ? "%lx" : "%lX";
        const uint64_t val = current_arg;

        switch (length_modifier) {
        case 2: { // hh - unsigned char
          unsigned char cval = static_cast<unsigned char>(current_arg & 0xFF);
          snprintf(buffer, sizeof(buffer), (spec == 'x') ? "%02x" : "%02X",
                   cval);
          break;
        }
        case 1: { // h - unsigned short
          unsigned short sval =
              static_cast<unsigned short>(current_arg & 0xFFFF);
          snprintf(buffer, sizeof(buffer), (spec == 'x') ? "%04x" : "%04X",
                   sval);
          break;
        }
        default: { // default - unsigned long
          snprintf(buffer, sizeof(buffer), fmt, val);
          break;
        }
        }
        output += buffer;
        break;
      }
      case 'c':
        output += static_cast<char>(current_arg & 0xFF);
        break;
      case 's':
        // String argument - address of another string
        {
          const uint64_t str_addr = current_arg;
          if (memory.is_mapped(str_addr)) {
            std::string str;
            uint64_t s_addr = str_addr;
            uint8_t s_byte;
            while ((s_byte = memory.read_byte(s_addr++)) != 0) {
              str += static_cast<char>(s_byte);
            }
            output += str;
          } else {
            output += "(null)";
          }
        }
        break;
      default:
        // Unknown specifier
        snprintf(buffer, sizeof(buffer), "<?>");
        output += buffer;
        break;
      }
      arg_idx++;
    } else {
      output += format[i];
    }
  }

  // Log the printf call details
  LOG_INFO("  -> printf: \"%s\"", format.c_str());
  if (expected_args > 0)
    LOG_INFO("  ->   arg1: 0x%lx (%ld)", arg1, arg1);
  if (expected_args > 1)
    LOG_INFO("  ->   arg2: 0x%lx (%ld)", arg2, arg2);

  // Output to stdout
  std::cout << output;
}

void Simulator::exec_br(const DecodedBranch &dec) {
  const uint64_t current_pc = regs.get_pc() - 8;
  regs.set_pc(dec.target);
  LOG_INFO("  -> BR: 0x%lx -> 0x%x", current_pc, dec.target);
}

void Simulator::exec_brcond(const DecodedCondBranch &dec) {
  const uint64_t cond_value = regs.read(dec.cond_reg);
  if (cond_value != 0) {
    regs.set_pc(dec.target);
    LOG_INFO("  -> BRCOND R%d (0x%lx): TRUE, branching to 0x%x", dec.cond_reg,
             cond_value, dec.target);
  } else {
    LOG_INFO("  -> BRCOND R%d (0x%lx): FALSE, continuing", dec.cond_reg,
             cond_value);
  }
}

void Simulator::dump_stack(uint64_t bytes) const {
  const uint64_t sp = regs.get_sp();
  const uint64_t fp = regs.get_fp();

  LOG_INFO("=== Stack Dump (SP=0x%lx, FP=0x%lx) ===", sp, fp);
  LOG_INFO("Address          Data            Interpretation");
  LOG_INFO("----------------------------------------");

  // Dump from SP - 32 to SP + bytes (to see both locals and incoming args)
  const uint64_t start = sp - 32;
  const uint64_t end = sp + bytes;

  for (uint64_t addr = start; addr < end; addr += 8) {
    if (!memory.is_mapped(addr)) {
      LOG_INFO("0x%016lx: ?? ?? ?? ?? ?? ?? ?? ??  (unmapped)", addr);
      continue;
    }

    uint64_t value = memory.read_qword(addr);

    // Mark special locations
    std::string marker = "";
    if (addr == sp)
      marker = "← SP";
    else if (addr == fp)
      marker = "← FP";
    else if (addr == fp - 8)
      marker = "← RA";
    else if (addr == fp - 16)
      marker = "← saved FP";

    // Check if value might be a code pointer
    if (value >= 0x11120 && value <= 0x113b0 && (value % 8 == 0)) {
      marker += " (code?)";
    }

    LOG_INFO(
        "0x%016lx: %02x %02x %02x %02x %02x %02x %02x %02x  = 0x%016lx  %s",
        addr, memory.read_byte(addr), memory.read_byte(addr + 1),
        memory.read_byte(addr + 2), memory.read_byte(addr + 3),
        memory.read_byte(addr + 4), memory.read_byte(addr + 5),
        memory.read_byte(addr + 6), memory.read_byte(addr + 7), value,
        marker.c_str());
  }
}

void Simulator::dump_stack_frame() const {
  const uint64_t fp = regs.get_fp();

  LOG_INFO("=== Stack Frame (FP=0x%lx) ===", fp);

  // Show saved registers
  if (memory.is_mapped(fp - 8)) {
    const uint64_t ra = memory.read_qword(fp - 8);
    LOG_INFO("  [FP - 8]  = 0x%016lx  (Return Address)", ra);
  }

  if (memory.is_mapped(fp - 16)) {
    const uint64_t old_fp = memory.read_qword(fp - 16);
    LOG_INFO("  [FP - 16] = 0x%016lx  (Saved FP)", old_fp);
  }

  // Show local variables (negative offsets)
  for (int offset = -24; offset >= -64; offset -= 8) {
    if (memory.is_mapped(fp + offset)) {
      const uint64_t val = memory.read_qword(fp + offset);
      LOG_INFO("  [FP %+d]  = 0x%016lx", offset, val);
    }
  }

  // Show incoming arguments (positive offsets)
  for (int offset = 8; offset <= 64; offset += 8) {
    if (memory.is_mapped(fp + offset)) {
      const uint64_t val = memory.read_qword(fp + offset);
      LOG_INFO("  [FP + %d]  = 0x%016lx", offset, val);
    }
  }
}

} // namespace srrarch