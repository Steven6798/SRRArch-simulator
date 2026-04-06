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

class Instruction {
public:
  // Construct from raw 64-bit little-endian value
  explicit Instruction(uint64_t raw_value);

  // Construct from 8-byte array
  explicit Instruction(const uint8_t *bytes);

  // Basic info
  __attribute__((always_inline)) inline Opcode opcode() const {
    return static_cast<Opcode>(raw & 0xFF);
  }
  __attribute__((always_inline)) inline uint64_t raw_value() const {
    return raw;
  }

  // Core register access (5 bits each)
  __attribute__((always_inline)) inline uint8_t reg1() const {
    return (raw >> 8) & 0x1F;
  } // bits 8-12
  __attribute__((always_inline)) inline uint8_t reg2() const {
    return (raw >> 13) & 0x1F;
  } // bits 13-17
  __attribute__((always_inline)) inline uint8_t reg3() const {
    return (raw >> 18) & 0x1F;
  } // bits 18-22

  // For immediate ALU operations (12-bit immediate at bit 18)
  __attribute__((always_inline)) inline int32_t alu_imm() const {
    uint32_t imm12 = (raw >> 18) & 0xFFF;

    // Shift left 20, then arithmetic shift right 20 to sign-extend
    return static_cast<int32_t>(imm12 << 20) >> 20;
  }

  // Arithmetic/Logical
  __attribute__((always_inline)) inline uint8_t arith_dest() const {
    return reg1();
  }
  __attribute__((always_inline)) inline uint8_t arith_src1() const {
    return reg2();
  }
  __attribute__((always_inline)) inline uint8_t arith_src2() const {
    return reg3();
  }
  __attribute__((always_inline)) inline int32_t arith_imm() const {
    return alu_imm();
  }

  // MOV
  __attribute__((always_inline)) inline uint8_t mov_dest() const {
    return reg1();
  }
  __attribute__((always_inline)) inline uint8_t mov_src() const {
    return reg2();
  }

  // For loads/stores
  __attribute__((always_inline)) inline uint8_t load_dest() const {
    return reg1();
  }
  __attribute__((always_inline)) inline uint8_t store_source() const {
    return reg1();
  }
  __attribute__((always_inline)) inline uint8_t mem_base() const {
    return reg2();
  }
  __attribute__((always_inline)) inline int32_t mem_offset() const {
    // 12-bit signed immediate at bits 18-29
    uint32_t imm12 = (raw >> 18) & 0xFFF;
    return static_cast<int32_t>(imm12 << 20) >> 20;
  }

  // RETURN
  __attribute__((always_inline)) inline uint8_t return_reg() const {
    return reg1();
  }

  // GENINT
  __attribute__((always_inline)) inline uint8_t genint_reg() const {
    return reg1();
  }
  __attribute__((always_inline)) inline uint32_t genint_imm() const {
    return (raw >> 13) & 0xFFFFFFFF;
  }

  // Comparison
  __attribute__((always_inline)) inline uint8_t cmp_dest() const {
    return reg1();
  }
  __attribute__((always_inline)) inline uint8_t cmp_src1() const {
    return reg2();
  }
  __attribute__((always_inline)) inline uint8_t cmp_src2() const {
    return reg3();
  }

  // Shift
  __attribute__((always_inline)) inline uint8_t shift_dest() const {
    return reg1();
  }
  __attribute__((always_inline)) inline uint8_t shift_src1() const {
    return reg2();
  }
  __attribute__((always_inline)) inline uint8_t shift_src2() const {
    return reg3();
  }
  __attribute__((always_inline)) inline int32_t shift_imm() const {
    return alu_imm();
  }

  // CALLREG (1 register: target address in register)
  __attribute__((always_inline)) inline uint8_t call_source() const {
    return reg1();
  }

  // For CALL - 32-bit absolute target address starting at bit 8
  __attribute__((always_inline)) inline uint32_t call_target() const {
    return (raw >> 8) & 0xFFFFFFFF;
  }

  // For BRCOND - 32-bit absolute target address starting at bit 13
  __attribute__((always_inline)) inline uint32_t brcond_target() const {
    return (raw >> 13) & 0xFFFFFFFF;
  }

  // For BRCOND - condition register
  __attribute__((always_inline)) inline uint8_t brcond_reg() const {
    return reg1();
  }

  // For BR - 32-bit absolute target address starting at bit 8
  __attribute__((always_inline)) inline uint32_t br_target() const {
    return (raw >> 8) & 0xFFFFFFFF;
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