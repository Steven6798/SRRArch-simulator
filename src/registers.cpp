/**
 * @file registers.cpp
 * @brief Register file implementation
 *
 * Implements read/write operations with bounds checking,
 * register name mapping, and debug dump functionality.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "registers.h"
#include "logger.h"
#include <iomanip>
#include <iostream>

namespace srrarch {

std::string register_to_string(uint8_t reg_num) {
  switch (reg_num) {
  case R0_PC:
    return "PC";
  case R1_SP:
    return "SP";
  case R2_FP:
    return "FP";
  case R3_RV:
    return "RV";
  case R5_ARG0:
    return "ARG0";
  case R6_ARG1:
    return "ARG1";
  case R7_ARG2:
    return "ARG2";
  case R8_ARG3:
    return "ARG3";
  case R9:
    return "R9";
  case R10:
    return "R10";
  case R11:
    return "R11";
  case R12:
    return "R12";
  case R13:
    return "R13";
  case R14:
    return "R14";
  case R15:
    return "R15";
  case R16:
    return "R16";
  case R17:
    return "R17";
  case R18:
    return "R18";
  case R19:
    return "R19";
  case R20:
    return "R20";
  case R21:
    return "R21";
  case R22:
    return "R22";
  case R23:
    return "R23";
  case R24:
    return "R24";
  case R25:
    return "R25";
  case R26:
    return "R26";
  case R27:
    return "R27";
  case R28:
    return "R28";
  case R29:
    return "R29";
  case R30:
    return "R30";
  case R31:
    return "R31";
  default:
    return "UNKNOWN";
  }
}

Registers::Registers() { reset(); }

uint64_t Registers::read(uint8_t reg_num) const {
  if (reg_num >= NUM_REGISTERS) {
    LOG_WARN("Attempt to read invalid register %d", static_cast<int>(reg_num));
    return 0;
  }
  return regs[reg_num];
}

void Registers::write(uint8_t reg_num, uint64_t value) {
  if (reg_num >= NUM_REGISTERS) {
    LOG_WARN("Attempt to write invalid register %d", static_cast<int>(reg_num));
    return;
  }
  regs[reg_num] = value;
}

void Registers::dump() const {
  std::cout << "\nRegister Dump:\n";
  std::cout << "==============\n";

  // Print in 4 columns for readability
  for (uint8_t i = 0; i < NUM_REGISTERS; i += 4) {
    for (uint8_t j = 0; j < 4 && (i + j) < NUM_REGISTERS; ++j) {
      uint8_t reg_num = i + j;
      std::cout << std::setw(4) << register_to_string(reg_num) << ": 0x"
                << std::hex << std::setw(16) << std::setfill('0')
                << regs[reg_num] << std::dec << std::setfill(' ');
      if (j < 3)
        std::cout << "  ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void Registers::reset() {
  for (int i = 0; i < NUM_REGISTERS; ++i) {
    regs[i] = 0;
  }
  // SP typically starts high and grows down, but we'll let the program set it
  // You might want to initialize SP to a default stack address here
}

} // namespace srrarch