#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <cstdint>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

struct SectionInfo {
  std::string name;
  uint64_t addr;
  uint64_t size;
  const uint8_t *data; // Pointer to section content in mapped file
};

class ElfLoader {
public:
  ElfLoader();
  ~ElfLoader();

  bool load(const char *filename);
  void unload();
  void *get_entry_point() const;
  const std::vector<SectionInfo> &get_executable_sections() const {
    return exec_sections;
  }

private:
  int fd;            // File descriptor (may be closed after mmap)
  struct stat st;    // File stat info (holds size)
  uint8_t *file_map; // Mapped file (kept until unload)
  Elf64_Ehdr *ehdr;  // Pointer to ELF header in mapped memory
  void *entry;       // Entry point address after loading
  bool is_loaded;
  std::vector<SectionInfo> exec_sections; // Stores executable sections

  bool load_segments();
  void parse_sections();
};

#endif // ELF_LOADER_H