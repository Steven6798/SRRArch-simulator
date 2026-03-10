#include "simulator.h"
#include "decoder.h"
#include "elf_loader.h"
#include <cassert>
#include <iomanip>
#include <iostream>

Simulator::Simulator()
    : entry_point(0), running(false), instruction_count(0), loader(nullptr) {
  regs.reset();
}

Simulator::~Simulator() {
  if (loader) {
    delete loader;
    loader = nullptr;
  }
}

bool Simulator::load_elf(const char *filename) {
  // Create and use ElfLoader to parse the ELF
  loader = new ElfLoader();

  if (!loader->load(filename)) {
    std::cerr << "Failed to load ELF file: " << filename << std::endl;
    delete loader;
    loader = nullptr;
    return false;
  }

  // Get entry point
  entry_point = reinterpret_cast<uint64_t>(loader->get_entry_point());
  regs.set_pc(entry_point);

  // Load executable sections into memory
  const auto &sections = loader->get_executable_sections();
  for (const auto &sec : sections) {
    std::cout << "Loading section " << sec.name << " at 0x" << std::hex
              << sec.addr << " (size: 0x" << sec.size << std::dec << ")"
              << std::endl;

    // Copy section data into simulator memory
    for (uint64_t i = 0; i < sec.size; i++) {
      memory[sec.addr + i] = sec.data[i];
    }
  }

  // Initialize stack pointer (typical location - can be adjusted)
  // For now, set SP to a high address (e.g., 0x80000000)
  regs.set_sp(0x80000000);

  return true;
}

uint64_t Simulator::fetch() {
  uint64_t pc = regs.get_pc();
  uint64_t instruction = 0;

  // Fetch 8 bytes from memory at PC
  for (int i = 0; i < 8; i++) {
    uint64_t addr = pc + i;
    auto it = memory.find(addr);
    if (it != memory.end()) {
      instruction |= static_cast<uint64_t>(it->second) << (i * 8);
    } else {
      std::cerr << "ERROR: Memory access violation at 0x" << std::hex << addr
                << std::dec << std::endl;
      running = false;
      return 0;
    }
  }

  // Advance PC
  regs.set_pc(pc + 8);

  return instruction;
}

void Simulator::execute(uint64_t instruction) {
  uint8_t opcode = instruction & 0xFF;

  // For debugging, print the instruction we're executing
  std::cout << "[" << std::dec << std::setw(6) << instruction_count << "] ";
  std::cout << "PC=0x" << std::hex << (regs.get_pc() - 8) << std::dec << ": ";

  // Decode and print (reuse your decoder)
  uint8_t bytes[8];
  for (int i = 0; i < 8; i++) {
    bytes[i] = (instruction >> (i * 8)) & 0xFF;
  }
  decode_instruction(bytes);

  switch (opcode) {
  case OP_NOP:
    exec_nop();
    break;

  case OP_RETURN: {
    uint8_t reg = (instruction >> 8) & 0x1F;
    exec_return(reg);
    break;
  }

  case OP_GENINT: {
    uint8_t reg = (instruction >> 8) & 0x1F;
    uint32_t imm = (instruction >> 13) & 0xFFFFFFFF;
    exec_genint(reg, imm);
    break;
  }

  case OP_SHL: {
    uint8_t dest = (instruction >> 8) & 0x1F;
    uint8_t src1 = (instruction >> 13) & 0x1F;
    uint8_t src2 = (instruction >> 18) & 0x1F;
    exec_shla(dest, src1, src2);
    break;
  }

  case OP_OR: {
    uint8_t dest = (instruction >> 8) & 0x1F;
    uint8_t src1 = (instruction >> 13) & 0x1F;
    uint8_t src2 = (instruction >> 18) & 0x1F;
    exec_or(dest, src1, src2);
    break;
  }

  case OP_MOV: {
    uint8_t dest = (instruction >> 8) & 0x1F;
    uint8_t src = (instruction >> 13) & 0x1F;
    exec_mov(dest, src);
    break;
  }

  default:
    std::cerr << "ERROR: Unknown opcode 0x" << std::hex
              << static_cast<int>(opcode) << std::dec << std::endl;
    running = false;
    break;
  }

  instruction_count++;
}

void Simulator::step() {
  if (!running)
    return;

  uint64_t instruction = fetch();
  if (running) {
    execute(instruction);
  }
}

void Simulator::run() {
  running = true;
  instruction_count = 0;

  std::cout << "\n=== Starting simulation ===" << std::endl;
  std::cout << "Entry point: 0x" << std::hex << entry_point << std::dec
            << std::endl;
  std::cout << "Initial register state:" << std::endl;
  regs.dump();

  while (running) {
    step();

    // Optional: add a break condition for infinite loops
    if (instruction_count > 1000) {
      std::cout << "Reached max instruction count (1000)" << std::endl;
      running = false;
    }
  }

  std::cout << "\n=== Simulation finished ===" << std::endl;
  std::cout << "Executed " << instruction_count << " instructions" << std::endl;
  std::cout << "Final register state:" << std::endl;
  regs.dump();
}

// Memory access methods
uint64_t Simulator::read_mem(uint64_t addr, size_t size) {
  uint64_t value = 0;
  for (size_t i = 0; i < size; i++) {
    auto it = memory.find(addr + i);
    if (it != memory.end()) {
      value |= static_cast<uint64_t>(it->second) << (i * 8);
    } else {
      std::cerr << "ERROR: Read from unmapped address 0x" << std::hex
                << (addr + i) << std::dec << std::endl;
      return 0;
    }
  }
  return value;
}

void Simulator::write_mem(uint64_t addr, uint64_t value, size_t size) {
  for (size_t i = 0; i < size; i++) {
    memory[addr + i] = (value >> (i * 8)) & 0xFF;
  }
}

// Instruction implementations
void Simulator::exec_nop() {
  // Do nothing
}

void Simulator::exec_return(uint8_t reg) {
  uint64_t retval = regs.read(reg);
  regs.set_retval(retval);
  std::cout << "  -> RETURN: halting with return value " << retval << std::endl;
  running = false; // Stop simulation
}

void Simulator::exec_genint(uint8_t reg, uint32_t imm) {
  // For now, just store the immediate in the register
  // Later, this could trigger system calls
  std::cout << "  -> GENINT: r" << static_cast<int>(reg) << " = 0x" << std::hex
            << imm << std::dec << std::endl;
  regs.write(reg, static_cast<uint64_t>(imm));
}

void Simulator::exec_shla(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t val1 = regs.read(src1);
  uint64_t val2 = regs.read(src2);
  uint64_t result = val1 << val2; // SHL (shift left)

  std::cout << "  -> SHLA: r" << static_cast<int>(dest) << " = r"
            << static_cast<int>(src1) << " << r" << static_cast<int>(src2)
            << " (0x" << std::hex << val1 << " << " << val2 << " = 0x" << result
            << std::dec << ")" << std::endl;

  regs.write(dest, result);
}

void Simulator::exec_or(uint8_t dest, uint8_t src1, uint8_t src2) {
  uint64_t val1 = regs.read(src1);
  uint64_t val2 = regs.read(src2);
  uint64_t result = val1 | val2;

  std::cout << "  -> OR: r" << static_cast<int>(dest) << " = r"
            << static_cast<int>(src1) << " | r" << static_cast<int>(src2)
            << " (0x" << std::hex << val1 << " | 0x" << val2 << " = 0x"
            << result << std::dec << ")" << std::endl;

  regs.write(dest, result);
}

void Simulator::exec_mov(uint8_t dest, uint8_t src) {
  uint64_t val = regs.read(src);

  std::cout << "  -> MOV: r" << static_cast<int>(dest) << " = r"
            << static_cast<int>(src) << " (0x" << std::hex << val << std::dec
            << ")" << std::endl;

  regs.write(dest, val);
}