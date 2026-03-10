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
  case Opcode::NOP:
    return 0;
  case Opcode::RETURN:
    return 1;
  case Opcode::GENINT:
    return 1;
  case Opcode::SHL:
    return 3;
  case Opcode::OR:
    return 3;
  case Opcode::MOV:
    return 2;
  default:
    return 0;
  }
}

std::string Instruction::to_string() const {
  std::stringstream ss;
  ss << opcode_to_string(opcode());

  switch (opcode()) {
  case Opcode::NOP:
    break;

  case Opcode::RETURN:
    ss << " R" << static_cast<int>(return_reg());
    break;

  case Opcode::GENINT:
    ss << " R" << static_cast<int>(genint_reg()) << ", 0x" << std::hex
       << immediate() << std::dec;
    break;

  case Opcode::SHL:
    ss << " R" << static_cast<int>(shl_dest()) << ", R"
       << static_cast<int>(shl_src1()) << ", R" << static_cast<int>(shl_src2());
    break;

  case Opcode::OR:
    ss << " R" << static_cast<int>(or_dest()) << ", R"
       << static_cast<int>(or_src1()) << ", R" << static_cast<int>(or_src2());
    break;

  case Opcode::MOV:
    ss << " R" << static_cast<int>(dest_reg()) << ", R"
       << static_cast<int>(src_reg());
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