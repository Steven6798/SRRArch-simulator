/**
 * @file decoder.cpp
 * @brief Implementation of instruction decoder
 *
 * Uses the Instruction class for all decoding.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "decoder.h"
#include "instruction.h"
#include "logger.h"

void decode_instruction(const uint8_t *inst) {
  Instruction instruction(inst);
  instruction.print_bytes();
  instruction.print();
}