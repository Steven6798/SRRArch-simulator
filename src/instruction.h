/**
 * @file instruction.h
 * @brief Instruction representation for SRRArch
 *
 * Encapsulates a 64-bit instruction with methods to extract
 * opcode, register fields, and immediates according to the
 * SRRArch instruction encoding.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "decoder.h" // For Opcode enum
#include <cstdint>
#include <string>

class Instruction {
public:
  // Construct from raw 64-bit little-endian value
  explicit Instruction(uint64_t raw_value);

  // Construct from 8-byte array
  explicit Instruction(const uint8_t *bytes);

  // Basic info
  Opcode opcode() const { return static_cast<Opcode>(raw & 0xFF); }
  uint64_t raw_value() const { return raw; }

  // Register fields (5 bits each)
  uint8_t reg1() const { return (raw >> 8) & 0x1F; }  // bits 8-12
  uint8_t reg2() const { return (raw >> 13) & 0x1F; } // bits 13-17
  uint8_t reg3() const { return (raw >> 18) & 0x1F; } // bits 18-22

  // For MOV (2 registers)
  uint8_t dest_reg() const { return reg1(); }
  uint8_t src_reg() const { return reg2(); }

  // For SHL/OR (3 registers)
  uint8_t shl_dest() const { return reg1(); }
  uint8_t shl_src1() const { return reg2(); }
  uint8_t shl_src2() const { return reg3(); }

  uint8_t or_dest() const { return reg1(); }
  uint8_t or_src1() const { return reg2(); }
  uint8_t or_src2() const { return reg3(); }

  // For RETURN (1 register)
  uint8_t return_reg() const { return reg1(); }

  // For GENINT (register + immediate)
  uint8_t genint_reg() const { return reg1(); }
  uint32_t immediate() const { return (raw >> 13) & 0xFFFFFFFF; }

  // Utility methods
  size_t register_count() const;
  std::string to_string() const;
  void print() const;

  // For debugging
  void print_bytes() const;

private:
  uint64_t raw; // Raw 64-bit instruction
};

#endif // INSTRUCTION_H