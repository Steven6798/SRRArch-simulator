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

  // Stores (0x12-0x15)
  STOREB = 0x12,
  STOREH = 0x13,
  STOREW = 0x14,
  STORE = 0x15,

  // Loads (0x16-0x1c)
  LOADBZ = 0x16,
  LOADBS = 0x17,
  LOADHZ = 0x18,
  LOADHS = 0x19,
  LOADWZ = 0x1a,
  LOADWS = 0x1b,
  LOAD = 0x1c,

  // Control flow (0x1d-0x20)
  RETURN = 0x1d,
  CALL = 0x1e,
  BRCOND = 0x1f,
  BR = 0x20,

  // Immediate and move (0x21-0x22)
  GENINT = 0x21,
  MOV = 0x22,

  COUNT
};

// Compile-time array of opcode names (index must match enum values)
constexpr const char *OPCODE_NAMES[] = {
    // Arithmetic/Logical
    "NOP", "ADD", "SUB", "MUL", "SDIV", "UDIV", "AND", "OR", "XOR", "SHL",
    "SRA", "SRL", "CMPEQ", "CMPNE", "CMPLT", "CMPGT", "CMPULT", "CMPUGT",
    // Stores
    "STOREB", "STOREH", "STOREW", "STORE",
    // Loads
    "LOADBZ", "LOADBS", "LOADHZ", "LOADHS", "LOADWZ", "LOADWS", "LOAD",
    // Control flow
    "RETURN", "CALL", "BRCOND", "BR",
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