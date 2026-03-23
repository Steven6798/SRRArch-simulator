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
  // Arithmetic/Logical (0x00-0x11)
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
  CMPULT = 0x10,
  CMPUGT = 0x11,

  // Arithmetic/Logical immediates (0x12-0x19)
  ADDI = 0x12,
  SUBI = 0x13,
  ANDI = 0x14,
  ORI = 0x15,
  XORI = 0x16,
  SHLI = 0x17,
  SRAI = 0x18,
  SRLI = 0x19,

  // Stores (0x1a-0x1d)
  STOREB = 0x1a,
  STOREH = 0x1b,
  STOREW = 0x1c,
  STORE = 0x1d,

  // Loads (0x1e-0x24)
  LOADBZ = 0x1e,
  LOADBS = 0x1f,
  LOADHZ = 0x20,
  LOADHS = 0x21,
  LOADWZ = 0x22,
  LOADWS = 0x23,
  LOAD = 0x24,

  // Control flow (0x25-0x29)
  RETURN = 0x25,
  CALL = 0x26,
  CALLREG = 0x27,
  BRCOND = 0x28,
  BR = 0x29,

  // Immediate and move (0x2a-0x2b)
  GENINT = 0x2a,
  MOV = 0x2b,

  COUNT
};

// Compile-time array of opcode names (index must match enum values)
constexpr const char *OPCODE_NAMES[] = {
    // Arithmetic/Logical
    "NOP", "ADD", "SUB", "MUL", "SDIV", "UDIV", "AND", "OR", "XOR", "SHL",
    "SRA", "SRL", "CMPEQ", "CMPNE", "CMPLT", "CMPGT", "CMPULT", "CMPUGT",
    // Arithmetic/Logical immediates
    "ADDI", "SUBI", "ANDI", "ORI", "XORI", "SHLI", "SRAI", "SRLI",
    // Stores
    "STOREB", "STOREH", "STOREW", "STORE",
    // Loads
    "LOADBZ", "LOADBS", "LOADHZ", "LOADHS", "LOADWZ", "LOADWS", "LOAD",
    // Control flow
    "RETURN", "CALL", "CALLREG", "BRCOND", "BR",
    // Immediate and move
    "GENINT", "MOV"};

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