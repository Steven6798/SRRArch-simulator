/**
 * @file instruction.cpp
 * @brief Instruction representation implementation
 *
 * Implements pre-decoding of instructions into type-specific structures
 * for fast execution.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "instruction.h"
#include "logger.h"
#include <iomanip>
#include <sstream>

namespace srrarch {

Instruction::Instruction(uint64_t raw_value) : raw(raw_value) {
  decode(raw_value);
}

Instruction::Instruction(const uint8_t *bytes) : raw(assemble_raw(bytes)) {
  decode(raw);
}

uint64_t Instruction::assemble_raw(const uint8_t *bytes) {
  uint64_t raw = 0;
  for (int i = 0; i < 8; ++i) {
    raw |= static_cast<uint64_t>(bytes[i]) << (i * 8);
  }
  return raw;
}

void Instruction::decode(uint64_t raw) {
  _opcode = static_cast<Opcode>(raw & 0xFF);

  switch (_opcode) {
  // 3-register operations (ADD, SUB, MUL, etc.)
  case Opcode::ADD:
  case Opcode::SUB:
  case Opcode::MUL:
  case Opcode::SDIV:
  case Opcode::UDIV:
  case Opcode::AND:
  case Opcode::OR:
  case Opcode::XOR:
  case Opcode::SHL:
  case Opcode::SRA:
  case Opcode::SRL:
  case Opcode::CMPEQ:
  case Opcode::CMPNE:
  case Opcode::CMPLT:
  case Opcode::CMPGT:
  case Opcode::CMPULT:
  case Opcode::CMPUGT:
    data.r.dest = (raw >> 8) & 0x1F;
    data.r.src1 = (raw >> 13) & 0x1F;
    data.r.src2 = (raw >> 18) & 0x1F;
    break;

  // 2-register + immediate operations
  case Opcode::ADDI:
  case Opcode::SUBI:
  case Opcode::ANDI:
  case Opcode::ORI:
  case Opcode::XORI:
  case Opcode::SHLI:
  case Opcode::SRAI:
  case Opcode::SRLI:
    data.ri.dest = (raw >> 8) & 0x1F;
    data.ri.src = (raw >> 13) & 0x1F;
    {
      uint32_t imm12 = (raw >> 18) & 0xFFF;
      data.ri.imm = static_cast<int32_t>(imm12 << 20) >> 20;
    }
    break;

  // Memory operations (LOAD/STORE family)
  case Opcode::LOADBZ:
  case Opcode::LOADBS:
  case Opcode::LOADHZ:
  case Opcode::LOADHS:
  case Opcode::LOADWZ:
  case Opcode::LOADWS:
  case Opcode::LOAD:
  case Opcode::STOREB:
  case Opcode::STOREH:
  case Opcode::STOREW:
  case Opcode::STORE:
    data.mem.reg = (raw >> 8) & 0x1F;
    data.mem.base = (raw >> 13) & 0x1F;
    {
      uint32_t imm12 = (raw >> 18) & 0xFFF;
      data.mem.offset = static_cast<int32_t>(imm12 << 20) >> 20;
    }
    break;

  // Unconditional branch
  case Opcode::BR:
    data.branch.target = (raw >> 8) & 0xFFFFFFFF;
    break;

  // Conditional branch
  case Opcode::BRCOND:
    data.cond_branch.cond_reg = (raw >> 8) & 0x1F;
    data.cond_branch.target = (raw >> 13) & 0xFFFFFFFF;
    break;

  // CALL (absolute)
  case Opcode::CALL:
    data.call.target = (raw >> 8) & 0xFFFFFFFF;
    break;

  // CALLREG (register indirect)
  case Opcode::CALLREG:
    data.callreg.target_reg = (raw >> 8) & 0x1F;
    break;

  // GENINT
  case Opcode::GENINT:
    data.genint.reg = (raw >> 8) & 0x1F;
    data.genint.imm = (raw >> 13) & 0xFFFFFFFF;
    break;

  // MOV
  case Opcode::MOV:
    data.mov.dest = (raw >> 8) & 0x1F;
    data.mov.src = (raw >> 13) & 0x1F;
    break;

  // SELECT
  case Opcode::SELECT:
    data.select.dest = (raw >> 8) & 0x1F;
    data.select.srcc = (raw >> 13) & 0x1F;
    data.select.srct = (raw >> 18) & 0x1F;
    data.select.srcf = (raw >> 23) & 0x1F;
    break;

  case Opcode::NOP:
  case Opcode::RETURN:
  default:
    // No operands to decode
    break;
  }
}

std::string Instruction::to_string() const {
  std::stringstream ss;
  ss << opcode_to_string(_opcode);

  switch (_opcode) {
  case Opcode::NOP:
  case Opcode::RETURN:
    break;

  case Opcode::GENINT:
    ss << " R" << static_cast<int>(data.genint.reg) << ", 0x" << std::hex
       << data.genint.imm << std::dec;
    break;

  case Opcode::MOV:
    ss << " R" << static_cast<int>(data.mov.dest) << ", R"
       << static_cast<int>(data.mov.src);
    break;

  case Opcode::LOADBZ:
  case Opcode::LOADBS:
  case Opcode::LOADHZ:
  case Opcode::LOADHS:
  case Opcode::LOADWZ:
  case Opcode::LOADWS:
  case Opcode::LOAD:
    ss << " R" << static_cast<int>(data.mem.reg) << ", R"
       << static_cast<int>(data.mem.base) << ", 0x" << std::hex
       << data.mem.offset << std::dec;
    break;

  case Opcode::STOREB:
  case Opcode::STOREH:
  case Opcode::STOREW:
  case Opcode::STORE:
    ss << " R" << static_cast<int>(data.mem.reg) << ", R"
       << static_cast<int>(data.mem.base) << ", 0x" << std::hex
       << data.mem.offset << std::dec;
    break;

  case Opcode::CALL:
    ss << " 0x" << std::hex << data.call.target << std::dec;
    break;

  case Opcode::CALLREG:
    ss << " R" << static_cast<int>(data.callreg.target_reg);
    break;

  // Arithmetic/Logical (3-register format)
  case Opcode::ADD:
  case Opcode::SUB:
  case Opcode::MUL:
  case Opcode::SDIV:
  case Opcode::UDIV:
  case Opcode::AND:
  case Opcode::OR:
  case Opcode::XOR:
    ss << " R" << static_cast<int>(data.r.dest) << ", R"
       << static_cast<int>(data.r.src1) << ", R"
       << static_cast<int>(data.r.src2);
    break;

  // Arithmetic/Logical register-immediate format
  case Opcode::ADDI:
  case Opcode::SUBI:
  case Opcode::ANDI:
  case Opcode::ORI:
  case Opcode::XORI:
    ss << " R" << static_cast<int>(data.ri.dest) << ", R"
       << static_cast<int>(data.ri.src) << ", 0x" << std::hex << data.ri.imm
       << std::dec;
    break;

  // Shifts (3-register format)
  case Opcode::SHL:
  case Opcode::SRA:
  case Opcode::SRL:
    ss << " R" << static_cast<int>(data.r.dest) << ", R"
       << static_cast<int>(data.r.src1) << ", R"
       << static_cast<int>(data.r.src2);
    break;

  // Shifts register-immediate format
  case Opcode::SHLI:
  case Opcode::SRAI:
  case Opcode::SRLI:
    ss << " R" << static_cast<int>(data.ri.dest) << ", R"
       << static_cast<int>(data.ri.src) << ", 0x" << std::hex << data.ri.imm
       << std::dec;
    break;

  // Comparisons (3-register format)
  case Opcode::CMPEQ:
  case Opcode::CMPNE:
  case Opcode::CMPLT:
  case Opcode::CMPGT:
  case Opcode::CMPULT:
  case Opcode::CMPUGT:
    ss << " R" << static_cast<int>(data.r.dest) << ", R"
       << static_cast<int>(data.r.src1) << ", R"
       << static_cast<int>(data.r.src2);
    break;

  case Opcode::BR:
    ss << " 0x" << std::hex << data.branch.target << std::dec;
    break;

  case Opcode::BRCOND:
    ss << " R" << static_cast<int>(data.cond_branch.cond_reg) << ", 0x"
       << std::hex << data.cond_branch.target << std::dec;
    break;

  case Opcode::SELECT:
    ss << " R" << static_cast<int>(data.select.dest) << ", R"
       << static_cast<int>(data.select.srcc) << ", R"
       << static_cast<int>(data.select.srct) << ", R"
       << static_cast<int>(data.select.srcf);
    break;

  default:
    ss << " (unknown)";
    break;
  }

  return ss.str();
}

void Instruction::print() const { LOG_INFO("%s", to_string().c_str()); }

void Instruction::print_bytes() const {
  std::stringstream ss;
  ss << "Bytes: ";
  for (int i = 0; i < 8; ++i) {
    ss << std::hex << std::setw(2) << std::setfill('0')
       << static_cast<int>((raw >> (i * 8)) & 0xFF) << " ";
  }
  LOG_INFO("%s", ss.str().c_str());
}

} // namespace srrarch