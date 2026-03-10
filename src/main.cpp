/**
 * @file main.cpp
 * @brief SRRArch simulator entry point
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "logger.h"
#include "simulator.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    LOG_ERROR("Usage: %s <elf-file>", argv[0]);
    return 1;
  }

  srrarch::Logger::instance().setLevel(srrarch::LogLevel::INFO);
  srrarch::Simulator sim;

  if (sim.load_elf(argv[1])) {
    sim.run();
    return 0;
  } else {
    LOG_ERROR("Failed to load ELF.");
    return 1;
  }
}