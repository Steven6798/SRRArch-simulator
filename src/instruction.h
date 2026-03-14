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

namespace srrarch {

class Instruction {
public:
  // Construct from raw 64-bit little-endian value
  explicit Instruction(uint64_t raw_value);

  // Construct from 8-byte array
  explicit Instruction(const uint8_t *bytes);

  // Basic info
  Opcode opcode() const { return static_cast<Opcode>(raw & 0xFF); }
  uint64_t raw_value() const { return raw; }

  // Core register access (5 bits each)
  uint8_t reg1() const { return (raw >> 8) & 0x1F; }  // bits 8-12
  uint8_t reg2() const { return (raw >> 13) & 0x1F; } // bits 13-17
  uint8_t reg3() const { return (raw >> 18) & 0x1F; } // bits 18-22

  // Arithmetic/Logical (3 registers: dest, src1, src2)
  uint8_t arith_dest() const { return reg1(); }
  uint8_t arith_src1() const { return reg2(); }
  uint8_t arith_src2() const { return reg3(); }

  // MOV (2 registers: dest, src)
  uint8_t mov_dest() const { return reg1(); }
  uint8_t mov_src() const { return reg2(); }

  // LOAD/STORE (2 registers: base, src/dest) - register indirect
  uint8_t load_reg() const { return reg1(); }  // Destination register
  uint8_t load_base() const { return reg2(); } // Base address register

  uint8_t store_base() const { return reg1(); } // Base address register
  uint8_t store_reg() const { return reg2(); }  // Source register

  // RETURN (1 register)
  uint8_t return_reg() const { return reg1(); }

  // GENINT (register + 32-bit immediate)
  uint8_t genint_reg() const { return reg1(); }
  uint32_t genint_imm() const { return (raw >> 13) & 0xFFFFFFFF; }

  // Comparison (3 registers: dest, src1, src2)
  uint8_t cmp_dest() const { return reg1(); }
  uint8_t cmp_src1() const { return reg2(); }
  uint8_t cmp_src2() const { return reg3(); }

  // Shift (3 registers: dest, src, shift_amount)
  uint8_t shift_dest() const { return reg1(); }
  uint8_t shift_src() const { return reg2(); }
  uint8_t shift_amount() const { return reg3(); }

  // CALL (1 register: target address in register)
  uint8_t call_target() const { return reg1(); }

  // For BRCOND - 32-bit absolute target address starting at bit 13
  uint32_t brcond_target() const {
    uint32_t imm = (raw >> 13) & 0xFFFFFFFF;
    return imm;
  }

  // For BRCOND - condition register
  uint8_t brcond_reg() const { return reg1(); }

  // For BR - 32-bit absolute target address starting at bit 8
  uint32_t br_target() const {
    uint32_t imm = (raw >> 8) & 0xFFFFFFFF;
    return imm;
  }

  // Utility
  size_t register_count() const;
  std::string to_string() const;
  void print() const;

  // For debugging
  void print_bytes() const;

private:
  uint64_t raw; // Raw 64-bit instruction
};

} // namespace srrarch

#endif // INSTRUCTION_H