/**
 * @file instruction.h
 * @brief Instruction representation for SRRArch
 *
 * Encapsulates a 64-bit instruction with pre-decoded fields for
 * fast execution using type-specific decoded structures.
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

// Type-specific decoded structures
struct DecodedR {
  uint8_t dest;
  uint8_t src1;
  uint8_t src2;
};

struct DecodedRI {
  uint8_t dest;
  uint8_t src;
  int32_t imm;
};

struct DecodedMem {
  uint8_t reg; // dest for LOAD, source for STORE
  uint8_t base;
  int32_t offset;
};

struct DecodedBranch {
  uint32_t target;
};

struct DecodedCondBranch {
  uint8_t cond_reg;
  uint32_t target;
};

struct DecodedCall {
  uint32_t target;
};

struct DecodedCallReg {
  uint8_t target_reg;
};

struct DecodedGenInt {
  uint8_t reg;
  uint32_t imm;
};

struct DecodedMov {
  uint8_t dest;
  uint8_t src;
};

class Instruction {
public:
  // Construct from raw 64-bit little-endian value
  explicit Instruction(uint64_t raw_value);

  // Construct from 8-byte array
  explicit Instruction(const uint8_t *bytes);

  // Basic info
  Opcode opcode() const { return _opcode; }
  uint64_t raw_value() const { return raw; }

  // Type-safe accessors
  const DecodedR &as_r() const { return data.r; }
  const DecodedRI &as_ri() const { return data.ri; }
  const DecodedMem &as_mem() const { return data.mem; }
  const DecodedBranch &as_branch() const { return data.branch; }
  const DecodedCondBranch &as_cond_branch() const { return data.cond_branch; }
  const DecodedCall &as_call() const { return data.call; }
  const DecodedCallReg &as_callreg() const { return data.callreg; }
  const DecodedGenInt &as_genint() const { return data.genint; }
  const DecodedMov &as_mov() const { return data.mov; }

  // Utility
  size_t register_count() const;
  std::string to_string() const;
  void print() const;
  void print_bytes() const;

private:
  uint64_t raw;
  Opcode _opcode;

  union DecodedData {
    DecodedR r;
    DecodedRI ri;
    DecodedMem mem;
    DecodedBranch branch;
    DecodedCondBranch cond_branch;
    DecodedCall call;
    DecodedCallReg callreg;
    DecodedGenInt genint;
    DecodedMov mov;

    DecodedData() {}
    ~DecodedData() {}
  } data;

  void decode(uint64_t raw);
  static uint64_t assemble_raw(const uint8_t *bytes);
};

} // namespace srrarch

#endif // INSTRUCTION_H