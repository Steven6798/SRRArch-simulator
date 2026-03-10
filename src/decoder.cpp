/**
 * @file decoder.cpp
 * @brief Implementation of instruction decoder
 *
 * Decodes 64-bit instructions into human-readable format.
 * Supports 6 instruction types: NOP, RETURN, GENINT, SHL, OR, MOV.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "decoder.h"
#include "logger.h"
#include <cstdint>
#include <iomanip>
#include <iostream>

std::string opcode_to_string(uint8_t op) {
  switch (op) {
  case OP_NOP:
    return "NOP";
  case OP_RETURN:
    return "RETURN";
  case OP_GENINT:
    return "GENINT";
  case OP_SHL:
    return "SHL";
  case OP_OR:
    return "OR";
  case OP_MOV:
    return "MOV";
  default:
    return "UNKNOWN";
  }
}

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

  uint8_t op = code & 0xFF;
  ss << " | Instruction: " << opcode_to_string(op);

  // Decode operands based on opcode
  switch (op) {
  case OP_NOP:
    break;

  case OP_RETURN: {
    uint8_t reg = (code >> 8) & 0x1F;
    ss << " R" << static_cast<int>(reg);
    break;
  }

  case OP_GENINT: {
    uint8_t reg = (code >> 8) & 0x1F;
    uint32_t imm = (code >> 13) & 0xFFFFFFFF;
    ss << " R" << static_cast<int>(reg) << ", 0x" << std::hex << imm
       << std::dec;
    break;
  }

  case OP_SHL:
  case OP_OR: {
    uint8_t reg1 = (code >> 8) & 0x1F;
    uint8_t reg2 = (code >> 13) & 0x1F;
    uint8_t reg3 = (code >> 18) & 0x1F;
    ss << " R" << static_cast<int>(reg1) << ", R" << static_cast<int>(reg2)
       << ", R" << static_cast<int>(reg3);
    break;
  }

  case OP_MOV: {
    uint8_t reg1 = (code >> 8) & 0x1F;
    uint8_t reg2 = (code >> 13) & 0x1F;
    ss << " R" << static_cast<int>(reg1) << ", R" << static_cast<int>(reg2);
    break;
  }

  default:
    break;
  }

  // Use LOG_INFO for instruction decoding output
  LOG_INFO("%s", ss.str().c_str());
}