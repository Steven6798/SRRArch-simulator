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
  RETURN = 0x01,
  GENINT = 0x02,
  SHL = 0x03,
  OR = 0x04,
  MOV = 0x05,
  COUNT // Automatically = 6, useful for bounds checking
};

// Compile-time array of opcode names (index must match enum values)
constexpr const char *OPCODE_NAMES[] = {
    "NOP",    // 0x00
    "RETURN", // 0x01
    "GENINT", // 0x02
    "SHL",    // 0x03
    "OR",     // 0x04
    "MOV"     // 0x05
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