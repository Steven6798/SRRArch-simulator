/**
 * @file decoder.cpp
 * @brief Implementation of instruction decoder
 *
 * Decodes 64-bit instructions into human-readable format.
 * Supports 6 instruction types: NOP, RETURN, GENINT, SHL, OR, MOV.
 * Register fields are 5 bits each, immediates are 32 bits.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "decoder.h"
#include "logger.h"
#include <cstdint>
#include <iomanip>
#include <sstream>

void decode_instruction(const uint8_t *inst) {
  std::stringstream ss;

  // Print raw hex bytes
  ss << "Bytes: ";
  for (int i = 0; i < 8; ++i) {
    ss << std::hex << std::setw(2) << std::setfill('0')
       << static_cast<int>(inst[i]) << " ";
  }
  ss << std::dec;

  // Assemble 64-bit little-endian value
  uint64_t code = 0;
  for (int i = 0; i < 8; ++i) {
    code |= static_cast<uint64_t>(inst[i]) << (i * 8);
  }

  Opcode op = static_cast<Opcode>(code & 0xFF);
  ss << " | Instruction: " << opcode_to_string(op);

  // Decode operands based on opcode
  switch (op) {
  case Opcode::NOP:
    break;

  case Opcode::RETURN: {
    uint8_t reg = (code >> 8) & 0x1F;
    ss << " R" << static_cast<int>(reg);
    break;
  }

  case Opcode::GENINT: {
    uint8_t reg = (code >> 8) & 0x1F;
    uint32_t imm = (code >> 13) & 0xFFFFFFFF;
    ss << " R" << static_cast<int>(reg) << ", 0x" << std::hex << imm
       << std::dec;
    break;
  }

  case Opcode::SHL:
  case Opcode::OR: {
    uint8_t reg1 = (code >> 8) & 0x1F;
    uint8_t reg2 = (code >> 13) & 0x1F;
    uint8_t reg3 = (code >> 18) & 0x1F;
    ss << " R" << static_cast<int>(reg1) << ", R" << static_cast<int>(reg2)
       << ", R" << static_cast<int>(reg3);
    break;
  }

  case Opcode::MOV: {
    uint8_t reg1 = (code >> 8) & 0x1F;
    uint8_t reg2 = (code >> 13) & 0x1F;
    ss << " R" << static_cast<int>(reg1) << ", R" << static_cast<int>(reg2);
    break;
  }

  default:
    // Unknown opcode – only raw bytes printed
    break;
  }

  LOG_INFO("%s", ss.str().c_str());
}