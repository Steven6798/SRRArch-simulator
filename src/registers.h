#ifndef REGISTERS_H
#define REGISTERS_H

#include <cstdint>
#include <string>

// Named registers for better code readability
enum Register : uint8_t {
  // Special purpose registers
  R0_PC = 0, // Program Counter
  R1_SP = 1, // Stack Pointer
  R2_FP = 2, // Frame Pointer
  R3_RV = 3, // Return Value

  // Argument registers
  R4_ARG0 = 4,
  R5_ARG1 = 5,
  R6_ARG2 = 6,
  R7_ARG3 = 7,

  // General purpose registers
  R8 = 8,
  R9 = 9,
  R10 = 10,
  R11 = 11,
  R12 = 12,
  R13 = 13,
  R14 = 14,
  R15 = 15,
  R16 = 16,
  R17 = 17,
  R18 = 18,
  R19 = 19,
  R20 = 20,
  R21 = 21,
  R22 = 22,
  R23 = 23,
  R24 = 24,
  R25 = 25,
  R26 = 26,
  R27 = 27,
  R28 = 28,
  R29 = 29,
  R30 = 30,
  R31 = 31,

  NUM_REGISTERS = 32
};

// Convert register number to its name string
std::string register_to_string(uint8_t reg_num);

class Registers {
public:
  Registers();

  // Read a register value (0-31)
  uint64_t read(uint8_t reg_num) const;

  // Write a register value (0-31)
  void write(uint8_t reg_num, uint64_t value);

  // Special getters/setters for named registers (for convenience)
  uint64_t get_pc() const { return regs[R0_PC]; }
  void set_pc(uint64_t value) { regs[R0_PC] = value; }

  uint64_t get_sp() const { return regs[R1_SP]; }
  void set_sp(uint64_t value) { regs[R1_SP] = value; }

  uint64_t get_fp() const { return regs[R2_FP]; }
  void set_fp(uint64_t value) { regs[R2_FP] = value; }

  uint64_t get_retval() const { return regs[R3_RV]; }
  void set_retval(uint64_t value) { regs[R3_RV] = value; }

  // Debug: print all register values
  void dump() const;

  // Reset all registers to 0
  void reset();

private:
  uint64_t regs[NUM_REGISTERS];
};

#endif // REGISTERS_H