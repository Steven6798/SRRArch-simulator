/**
 * @file instruction.cpp
 * @brief Instruction representation implementation
 *
 * Implements field extraction and string formatting for
 * SRRArch instructions.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "instruction.h"
#include "logger.h"
#include <iomanip>
#include <iostream>
#include <sstream>

namespace srrarch {

Instruction::Instruction(uint64_t raw_value) : raw(raw_value) {}

Instruction::Instruction(const uint8_t *bytes) : raw(0) {
  // Assemble 64-bit little-endian value
  for (int i = 0; i < 8; ++i) {
    raw |= static_cast<uint64_t>(bytes[i]) << (i * 8);
  }
}

size_t Instruction::register_count() const {
  switch (opcode()) {
  // 0 registers
  case Opcode::NOP:
  case Opcode::RETURN:
  case Opcode::BR:
    return 0;

  // 1 register
  case Opcode::GENINT:
  case Opcode::CALL:
  case Opcode::BRCOND:
    return 1;

  // 2 registers
  case Opcode::MOV:
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
    return 2;

  // 3 registers (most arithmetic/logic/shift/compare)
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
    return 3;

  default:
    return 0;
  }
}

std::string Instruction::to_string() const {
  std::stringstream ss;
  ss << opcode_to_string(opcode());

  switch (opcode()) {
  case Opcode::NOP:
  case Opcode::RETURN:
    break;

  case Opcode::GENINT:
    ss << " R" << static_cast<int>(genint_reg()) << ", 0x" << std::hex
       << genint_imm() << std::dec;
    break;

  case Opcode::MOV:
    ss << " R" << static_cast<int>(mov_dest()) << ", R"
       << static_cast<int>(mov_src());
    break;

  case Opcode::LOADBZ:
  case Opcode::LOADBS:
  case Opcode::LOADHZ:
  case Opcode::LOADHS:
  case Opcode::LOADWZ:
  case Opcode::LOADWS:
  case Opcode::LOAD:
    ss << " R" << static_cast<int>(load_dest()) << ", R"
       << static_cast<int>(load_base());
    break;

  case Opcode::STOREB:
  case Opcode::STOREH:
  case Opcode::STOREW:
  case Opcode::STORE:
    ss << " R" << static_cast<int>(store_base()) << ", R"
       << static_cast<int>(store_source());
    break;

  case Opcode::CALL:
    ss << " R" << static_cast<int>(call_target());
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
    ss << " R" << static_cast<int>(arith_dest()) << ", R"
       << static_cast<int>(arith_src1()) << ", R"
       << static_cast<int>(arith_src2());
    break;

  // Shifts (3-register format)
  case Opcode::SHL:
  case Opcode::SRA:
  case Opcode::SRL:
    ss << " R" << static_cast<int>(shift_dest()) << ", R"
       << static_cast<int>(shift_src()) << ", R"
       << static_cast<int>(shift_amount());
    break;

  // Comparisons (3-register format)
  case Opcode::CMPEQ:
  case Opcode::CMPNE:
  case Opcode::CMPLT:
  case Opcode::CMPGT:
    ss << " R" << static_cast<int>(cmp_dest()) << ", R"
       << static_cast<int>(cmp_src1()) << ", R" << static_cast<int>(cmp_src2());
    break;

  case Opcode::BR:
    ss << " 0x" << std::hex << br_target() << std::dec;
    break;
  case Opcode::BRCOND:
    ss << " R" << static_cast<int>(brcond_reg()) << ", 0x" << std::hex
       << brcond_target() << std::dec;
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