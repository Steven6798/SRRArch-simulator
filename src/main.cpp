#include "simulator.h"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <elf-file>" << std::endl;
    return 1;
  }

  Simulator sim;

  if (sim.load_elf(argv[1])) {
    sim.run();
    return 0;
  } else {
    std::cerr << "Failed to load ELF." << std::endl;
    return 1;
  }
}