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
  inline Opcode opcode() const { return static_cast<Opcode>(raw & 0xFF); }
  inline uint64_t raw_value() const { return raw; }

  // Core register access (5 bits each)
  inline uint8_t reg1() const { return (raw >> 8) & 0x1F; }  // bits 8-12
  inline uint8_t reg2() const { return (raw >> 13) & 0x1F; } // bits 13-17
  inline uint8_t reg3() const { return (raw >> 18) & 0x1F; } // bits 18-22

  // For immediate ALU operations (12-bit immediate at bit 18)
  int32_t alu_imm() const {
    uint32_t imm12 = (raw >> 18) & 0xFFF;

    // Shift left 20, then arithmetic shift right 20 to sign-extend
    return static_cast<int32_t>(imm12 << 20) >> 20;
  }

  // Arithmetic/Logical
  inline uint8_t arith_dest() const { return reg1(); }
  inline uint8_t arith_src1() const { return reg2(); }
  inline uint8_t arith_src2() const { return reg3(); }
  inline int32_t arith_imm() const { return alu_imm(); }

  // MOV
  inline uint8_t mov_dest() const { return reg1(); }
  inline uint8_t mov_src() const { return reg2(); }

  // For loads/stores
  inline uint8_t load_dest() const { return reg1(); }
  inline uint8_t store_source() const { return reg1(); }
  inline uint8_t mem_base() const { return reg2(); }
  inline int32_t mem_offset() const {
    // 12-bit signed immediate at bits 18-29
    uint32_t imm12 = (raw >> 18) & 0xFFF;
    return static_cast<int32_t>(imm12 << 20) >> 20;
  }

  // RETURN
  inline uint8_t return_reg() const { return reg1(); }

  // GENINT
  inline uint8_t genint_reg() const { return reg1(); }
  inline uint32_t genint_imm() const { return (raw >> 13) & 0xFFFFFFFF; }

  // Comparison
  inline uint8_t cmp_dest() const { return reg1(); }
  inline uint8_t cmp_src1() const { return reg2(); }
  inline uint8_t cmp_src2() const { return reg3(); }

  // Shift
  inline uint8_t shift_dest() const { return reg1(); }
  inline uint8_t shift_src1() const { return reg2(); }
  inline uint8_t shift_src2() const { return reg3(); }
  inline int32_t shift_imm() const { return alu_imm(); }

  // CALLREG (1 register: target address in register)
  inline uint8_t call_source() const { return reg1(); }

  // For CALL - 32-bit absolute target address starting at bit 8
  inline uint32_t call_target() const { return (raw >> 8) & 0xFFFFFFFF; }

  // For BRCOND - 32-bit absolute target address starting at bit 13
  inline uint32_t brcond_target() const { return (raw >> 13) & 0xFFFFFFFF; }

  // For BRCOND - condition register
  inline uint8_t brcond_reg() const { return reg1(); }

  // For BR - 32-bit absolute target address starting at bit 8
  inline uint32_t br_target() const { return (raw >> 8) & 0xFFFFFFFF; }

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