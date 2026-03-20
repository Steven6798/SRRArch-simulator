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
    LOG_INFO("Loading data section %s at 0x%lx (size: 0x%lx bytes)",
             sec.name.c_str(), sec.addr, sec.size);
    memory.load_segment(sec.addr, sec.data, sec.size);
    total_bytes += sec.size;
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
  uint64_t pc = regs.get_pc();

  // Use Memory class to read instruction
  uint64_t instruction = memory.read_qword(pc);
  LOG_DBG("Fetched instruction at 0x%lx: 0x%016lx", pc, instruction);

  // Advance PC
  regs.set_pc(pc + 8);
  return instruction;
}

void Simulator::execute(const Instruction &inst) {
  // Validate opcode before execution
  uint8_t op = static_cast<uint8_t>(inst.opcode());
  if (op >= static_cast<uint8_t>(Opcode::COUNT)) {
    LOG_ERROR("Invalid opcode 0x%02x at PC=0x%lx", op, regs.get_pc() - 8);
    LOG_ERROR("  Raw instruction: 0x%016lx", inst.raw_value());
    LOG_ERROR("  Halting simulation");
    running = false;
    return;
  }

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
  case Opcode::CMPULT:
    exec_cmpult(inst.cmp_dest(), inst.cmp_src1(), inst.cmp_src2());
    break;
  case Opcode::CMPUGT:
    exec_cmpugt(inst.cmp_dest(), inst.cmp_src1(), inst.cmp_src2());
    break;

  // Memory
  case Opcode::STOREB:
    exec_storeb(inst.store_base(), inst.store_source());
    break;
  case Opcode::STOREH:
    exec_storeh(inst.store_base(), inst.store_source());
    break;
  case Opcode::STOREW:
    exec_storew(inst.store_base(), inst.store_source());
    break;
  case Opcode::STORE:
    exec_store(inst.store_base(), inst.store_source());
    break;

  case Opcode::LOADBZ:
    exec_loadbz(inst.load_dest(), inst.load_base());
    break;
  case Opcode::LOADBS:
    exec_loadbs(inst.load_dest(), inst.load_base());
    break;
  case Opcode::LOADHZ:
    exec_loadhz(inst.load_dest(), inst.load_base());
    break;
  case Opcode::LOADHS:
    exec_loadhs(inst.load_dest(), inst.load_base());
    break;
  case Opcode::LOADWZ:
    exec_loadwz(inst.load_dest(), inst.load_base());
    break;
  case Opcode::LOADWS:
    exec_loadws(inst.load_dest(), inst.load_base());
    break;
  case Opcode::LOAD:
    exec_load(inst.load_dest(), inst.load_base());
    break;

  case Opcode::CALL:
    exec_call(inst.call_target());
    break;

  case Opcode::BR:
    exec_br(inst.br_target());
    break;
  case Opcode::BRCOND:
    exec_brcond(inst.brcond_reg(), inst.brcond_target());
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
  //   regs.dump();

  while (running) {
    step();
    if (instruction_count >= max_instructions) {
      LOG_WARN("Reached max instruction count (%lu)", max_instructions);
      running = false;
    }
  }

  LOG_INFO("=== Simulation finished ===");
  LOG_INFO("Executed %lu instructions", instruction_count);
  LOG_DBG("Final register state:");
  //   regs.dump();
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

void Simulator::exec_cmpult(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t a = static_cast<uint64_t>(regs.read(src1));
  uint64_t b = static_cast<uint64_t>(regs.read(src2));
  uint64_t result = (a < b) ? 1 : 0;
  LOG_INFO("  -> CMPULT: R%u = (R%u < R%u) ? 1 : 0 (%lu < %lu = %lu)", dest,
           src1, src2, a, b, result);
  regs.write(dest, result);
}

void Simulator::exec_cmpugt(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t a = static_cast<uint64_t>(regs.read(src1));
  uint64_t b = static_cast<uint64_t>(regs.read(src2));
  uint64_t result = (a > b) ? 1 : 0;
  LOG_INFO("  -> CMPUGT: R%u = (R%u > R%u) ? 1 : 0 (%lu > %lu = %lu)", dest,
           src1, src2, a, b, result);
  regs.write(dest, result);
}

// Memory operations
void Simulator::exec_storeb(uint8_t base, uint8_t src) {
  uint64_t addr = regs.read(base);
  uint64_t value = regs.read(src);

  memory.write_byte(addr, value & 0xFF);
  LOG_INFO("  -> STOREB: [R%u] = R%u (0x%lx = 0x%02llx)", base, src, addr,
           value & 0xFF);
}

void Simulator::exec_storeh(uint8_t base, uint8_t src) {
  uint64_t addr = regs.read(base);
  uint64_t value = regs.read(src);

  memory.write_word(addr, value & 0xFFFF);
  LOG_INFO("  -> STOREH: [R%u] = R%u (0x%lx = 0x%04llx)", base, src, addr,
           value & 0xFFFF);
}

void Simulator::exec_storew(uint8_t base, uint8_t src) {
  uint64_t addr = regs.read(base);
  uint64_t value = regs.read(src);

  memory.write_dword(addr, value & 0xFFFFFFFF);
  LOG_INFO("  -> STOREW: [R%u] = R%u (0x%lx = 0x%08llx)", base, src, addr,
           value & 0xFFFFFFFF);
}

void Simulator::exec_store(uint8_t base, uint8_t src) {
  uint64_t addr = regs.read(base);
  uint64_t value = regs.read(src);

  memory.write_qword(addr, value);
  LOG_INFO("  -> STORE: [R%u] = R%u (0x%lx = 0x%lx)", base, src, addr, value);
}

void Simulator::exec_loadbz(uint8_t dest, uint8_t base) {
  uint64_t addr = regs.read(base);
  uint8_t value = memory.read_byte(addr);

  regs.write(dest, value);
  LOG_INFO("  -> LOADBZ: R%u = [R%u] (0x%lx = 0x%02llx)", dest, base, addr,
           value);
}

void Simulator::exec_loadbs(uint8_t dest, uint8_t base) {
  uint64_t addr = regs.read(base);
  int8_t value = static_cast<int8_t>(memory.read_byte(addr));

  regs.write(dest, static_cast<uint64_t>(value));
  LOG_INFO("  -> LOADBS: R%u = [R%u] (0x%lx = 0x%02llx -> 0x%016llx)", dest,
           base, addr, static_cast<uint8_t>(value), regs.read(dest));
}

void Simulator::exec_loadhz(uint8_t dest, uint8_t base) {
  uint64_t addr = regs.read(base);
  uint16_t value = memory.read_word(addr);

  regs.write(dest, value);
  LOG_INFO("  -> LOADHZ: R%u = [R%u] (0x%lx = 0x%04llx)", dest, base, addr,
           value);
}

void Simulator::exec_loadhs(uint8_t dest, uint8_t base) {
  uint64_t addr = regs.read(base);
  int16_t value = static_cast<int16_t>(memory.read_word(addr));

  regs.write(dest, static_cast<uint64_t>(value));
  LOG_INFO("  -> LOADHS: R%u = [R%u] (0x%lx = 0x%04llx -> 0x%016llx)", dest,
           base, addr, static_cast<uint16_t>(value), regs.read(dest));
}

void Simulator::exec_loadwz(uint8_t dest, uint8_t base) {
  uint64_t addr = regs.read(base);
  uint32_t value = memory.read_dword(addr);

  regs.write(dest, value);
  LOG_INFO("  -> LOADWZ: R%u = [R%u] (0x%lx = 0x%08llx)", dest, base, addr,
           value);
}

void Simulator::exec_loadws(uint8_t dest, uint8_t base) {
  uint64_t addr = regs.read(base);
  int32_t value = static_cast<int32_t>(memory.read_dword(addr));

  regs.write(dest, static_cast<uint64_t>(value));
  LOG_INFO("  -> LOADWS: R%u = [R%u] (0x%lx = 0x%08llx -> 0x%016llx)", dest,
           base, addr, static_cast<uint32_t>(value), regs.read(dest));
}

void Simulator::exec_load(uint8_t dest, uint8_t base) {
  uint64_t addr = regs.read(base);
  uint64_t value = memory.read_qword(addr);

  regs.write(dest, value);
  LOG_INFO("  -> LOAD: R%u = [R%u] (0x%lx = 0x%lx)", dest, base, addr, value);
}

void Simulator::exec_call(uint8_t target_reg) {
  // dump_stack_frame();
  uint64_t target = regs.read(target_reg);
  uint64_t return_addr = regs.get_pc(); // PC already advanced by fetch()

  // Simple check for printf
  if (loader && loader->is_printf_undefined() && target == 0) {
    LOG_INFO("  -> CALL to printf detected");
    exec_printf();
    return;
  }

  // Validate target address
  if (!memory.is_mapped(target)) {
    LOG_ERROR("  -> CALL R%u: target address 0x%lx is not mapped in memory!",
              target_reg, target);
    LOG_ERROR("  -> Halting simulation due to invalid call target");
    running = false;
    return;
  }

  // Save return address to link register (R4)
  regs.write(RA_REG, return_addr);

  // Jump to target
  regs.set_pc(target);

  LOG_INFO("  -> CALL R%u (0x%lx), return addr 0x%lx saved to R4", target_reg,
           target, return_addr);
}

void Simulator::exec_printf() {
  // All printf calls use the same argument access pattern
  uint64_t args_base = regs.get_sp();
  if (!memory.is_mapped(args_base)) {
    LOG_ERROR("  -> printf error: args_base 0x%lx not mapped!", args_base);
    running = false;
    return;
  }

  uint64_t format_addr = memory.read_qword(args_base);
  uint64_t arg1 = memory.read_qword(args_base + 8);
  uint64_t arg2 = memory.read_qword(args_base + 16);
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
      i++; // Skip the specifier

      if (spec == '%') {
        output += '%'; // Escaped percent
      } else {
        // Get the appropriate argument
        uint64_t current_arg = 0;
        if (arg_idx == 0)
          current_arg = arg1;
        else if (arg_idx == 1)
          current_arg = arg2;

        // Handle 'l' modifier by just skipping it
        if (spec == 'l' && i + 1 < format.length()) {
          spec = format[i + 1]; // Get the actual specifier
          i++;                  // Skip the 'l'
        }

        // Format based on specifier (all integers are 64-bit)
        char buffer[32];
        switch (spec) {
        case 'd':
        case 'i':
          snprintf(buffer, sizeof(buffer), "%ld",
                   static_cast<long>(current_arg));
          output += buffer;
          break;
        case 'u':
          snprintf(buffer, sizeof(buffer), "%lu",
                   static_cast<unsigned long>(current_arg));
          output += buffer;
          break;
        case 'x':
        case 'X':
          snprintf(buffer, sizeof(buffer), (spec == 'x') ? "%lx" : "%lX",
                   current_arg);
          output += buffer;
          break;
        case 'c':
          output += static_cast<char>(current_arg & 0xFF);
          break;
        case 's':
          // String argument - address of another string
          {
            uint64_t str_addr = current_arg;
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
      }
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

void Simulator::exec_br(uint32_t target_addr) {
  uint64_t current_pc = regs.get_pc() - 8;

  // Set PC to absolute target
  regs.set_pc(target_addr);

  LOG_INFO("  -> BR: 0x%lx -> 0x%x", current_pc, target_addr);
}

void Simulator::exec_brcond(uint8_t cond_reg, uint32_t target_addr) {
  uint64_t cond_value = regs.read(cond_reg);

  if (cond_value != 0) {
    // Condition true - branch to absolute address
    regs.set_pc(target_addr);
    LOG_INFO("  -> BRCOND R%d (0x%lx): TRUE, branching to 0x%x", cond_reg,
             cond_value, target_addr);
  } else {
    // Condition false - continue
    LOG_INFO("  -> BRCOND R%d (0x%lx): FALSE, continuing", cond_reg,
             cond_value);
  }
}

void Simulator::dump_stack(uint64_t bytes) const {
  uint64_t sp = regs.get_sp();
  uint64_t fp = regs.get_fp();

  LOG_INFO("=== Stack Dump (SP=0x%lx, FP=0x%lx) ===", sp, fp);
  LOG_INFO("Address          Data            Interpretation");
  LOG_INFO("----------------------------------------");

  // Dump from SP - 32 to SP + bytes (to see both locals and incoming args)
  uint64_t start = sp - 32;
  uint64_t end = sp + bytes;

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
  uint64_t fp = regs.get_fp();

  LOG_INFO("=== Stack Frame (FP=0x%lx) ===", fp);

  // Show saved registers
  if (memory.is_mapped(fp - 8)) {
    uint64_t ra = memory.read_qword(fp - 8);
    LOG_INFO("  [FP - 8]  = 0x%016lx  (Return Address)", ra);
  }

  if (memory.is_mapped(fp - 16)) {
    uint64_t old_fp = memory.read_qword(fp - 16);
    LOG_INFO("  [FP - 16] = 0x%016lx  (Saved FP)", old_fp);
  }

  // Show local variables (negative offsets)
  for (int offset = -24; offset >= -64; offset -= 8) {
    if (memory.is_mapped(fp + offset)) {
      uint64_t val = memory.read_qword(fp + offset);
      LOG_INFO("  [FP %+d]  = 0x%016lx", offset, val);
    }
  }

  // Show incoming arguments (positive offsets)
  for (int offset = 8; offset <= 64; offset += 8) {
    if (memory.is_mapped(fp + offset)) {
      uint64_t val = memory.read_qword(fp + offset);
      LOG_INFO("  [FP + %d]  = 0x%016lx", offset, val);
    }
  }
}

} // namespace srrarch