/**
 * @file decoder.h
 * @brief Instruction decoder for SRRArch custom architecture
 *
 * Uses the Instruction class for all decoding.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#ifndef DECODER_H
#define DECODER_H

#include <cstdint>
#include <string>

namespace srrarch {

// Custom opcodes as enum class for type safety
enum class Opcode : uint8_t {
  NOP = 0x00,
  ADD = 0x01,
  SUB = 0x02,
  MUL = 0x03,
  SDIV = 0x04,
  UDIV = 0x05,
  AND = 0x06,
  OR = 0x07,
  XOR = 0x08,
  SHL = 0x09,
  SRA = 0x0a,
  SRL = 0x0b,
  CMPEQ = 0x0c,
  CMPNE = 0x0d,
  CMPLT = 0x0e,
  CMPGT = 0x0f,
  STORE = 0x10,
  LOAD = 0x11,
  RETURN = 0x12,
  GENINT = 0x13,
  MOV = 0x14,
  CALL = 0x15,
  COUNT // useful for bounds checking
};

// Compile-time array of opcode names (index must match enum values)
constexpr const char *OPCODE_NAMES[] = {
    "NOP",    // 0x00
    "ADD",    // 0x01
    "SUB",    // 0x02
    "MUL",    // 0x03
    "SDIV",   // 0x04
    "UDIV",   // 0x05
    "AND",    // 0x06
    "OR",     // 0x07
    "XOR",    // 0x08
    "SHL",    // 0x09
    "SRA",    // 0x0a
    "SRL",    // 0x0b
    "CMPEQ",  // 0x0c
    "CMPNE",  // 0x0d
    "CMPLT",  // 0x0e
    "CMPGT",  // 0x0f
    "STORE",  // 0x10
    "LOAD",   // 0x11
    "RETURN", // 0x12
    "GENINT", // 0x13
    "MOV",    // 0x14
    "CALL"    // 0x15
};

// Compile-time assertion that COUNT matches array size
static_assert(static_cast<size_t>(Opcode::COUNT) ==
                  sizeof(OPCODE_NAMES) / sizeof(OPCODE_NAMES[0]),
              "Opcode::COUNT must match OPCODE_NAMES array size");

// Convert opcode to string at compile time
constexpr const char *opcode_to_string(Opcode op) {
  return OPCODE_NAMES[static_cast<size_t>(op)];
}

// Decode one 8-byte instruction from an executable section
void decode_instruction(const uint8_t *inst);

} // namespace srrarch

#endif // DECODER_H