#include "decoder.h"
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
  // Print raw hex bytes (little-endian order: first byte is LSB)
  std::cout << "Bytes: ";
  for (int i = 0; i < 8; ++i) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(inst[i]) << " ";
  }
  std::cout << std::dec;

  // Assemble 64-bit little-endian value
  uint64_t code = 0;
  for (int i = 0; i < 8; ++i) {
    code |= static_cast<uint64_t>(inst[i]) << (i * 8);
  }

  uint8_t op = code & 0xFF; // opcode in low byte
  std::cout << " | Instruction: " << opcode_to_string(op);

  // Decode operands based on opcode
  switch (op) {
  case OP_NOP:
    // No operands
    break;

  case OP_RETURN: {
    // One register in bits 8-12
    uint8_t reg = (code >> 8) & 0x1F;
    std::cout << " R" << static_cast<int>(reg);
    break;
  }

  case OP_GENINT: {
    // Register in bits 8-12, 32-bit immediate in bits 13-44
    uint8_t reg = (code >> 8) & 0x1F;
    uint32_t imm = (code >> 13) & 0xFFFFFFFF;
    std::cout << " R" << static_cast<int>(reg) << ", 0x" << std::hex << imm
              << std::dec;
    break;
  }

  case OP_SHL:
  case OP_OR: {
    // Three registers: bits 8-12, 13-17, 18-22
    uint8_t reg1 = (code >> 8) & 0x1F;
    uint8_t reg2 = (code >> 13) & 0x1F;
    uint8_t reg3 = (code >> 18) & 0x1F;
    std::cout << " R" << static_cast<int>(reg1) << ", R"
              << static_cast<int>(reg2) << ", R" << static_cast<int>(reg3);
    break;
  }

  case OP_MOV: {
    // Two registers: bits 8-12, 13-17
    uint8_t reg1 = (code >> 8) & 0x1F;
    uint8_t reg2 = (code >> 13) & 0x1F;
    std::cout << " R" << static_cast<int>(reg1) << ", R"
              << static_cast<int>(reg2);
    break;
  }

  default:
    // Unknown opcode – only raw bytes printed
    break;
  }
  std::cout << std::endl;
}