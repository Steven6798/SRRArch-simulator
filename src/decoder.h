#ifndef DECODER_H
#define DECODER_H

#include <cstdint>
#include <string>

// Custom opcodes
enum Opcode : uint8_t {
  OP_NOP = 0x00,
  OP_RETURN = 0x01,
  OP_GENINT = 0x02,
  OP_SHL = 0x03,
  OP_OR = 0x04,
  OP_MOV = 0x05
};

// Convert opcode to string
std::string opcode_to_string(uint8_t op);

// Decode one 8-byte instruction from an executable section
void decode_instruction(const uint8_t *inst);

#endif // DECODER_H