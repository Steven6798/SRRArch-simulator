/**
 * @file elf_loader.h
 * @brief ELF file loader for SRRArch simulator
 *
 * Provides functionality to parse 64-bit ELF executable files,
 * extract program headers, section headers, and load executable
 * sections into memory. Supports little-endian ELF format.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

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

namespace srrarch {

enum class LoadResult {
  SUCCESS,            ///< ELF loaded successfully
  FILE_NOT_FOUND,     ///< File doesn't exist or can't be opened
  STAT_FAILED,        ///< fstat() failed
  INVALID_FILE_SIZE,  ///< File size is negative or zero
  MMAP_FAILED,        ///< mmap() failed to map file
  INVALID_ELF_MAGIC,  ///< Not a valid ELF file
  UNSUPPORTED_ARCH,   ///< Not 64-bit
  UNSUPPORTED_ENDIAN, ///< Not little-endian
  NO_SECTIONS,        ///< No section headers found
  CORRUPT_SECTION,    ///< Section data out of bounds
  SEGMENT_LOAD_FAILED ///< Failed to load a program segment
};

const char *load_result_to_string(LoadResult result);

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

  LoadResult load(const char *filename);
  void unload();
  void *get_entry_point() const;
  const std::vector<SectionInfo> &get_executable_sections() const {
    return exec_sections;
  }
  const std::vector<SectionInfo> &get_data_sections() const {
    return data_sections;
  }

private:
  int fd;            // File descriptor (may be closed after mmap)
  struct stat st;    // File stat info (holds size)
  uint8_t *file_map; // Mapped file (kept until unload)
  Elf64_Ehdr *ehdr;  // Pointer to ELF header in mapped memory
  void *entry;       // Entry point address after loading
  bool is_loaded;
  std::vector<SectionInfo> exec_sections; // Stores executable sections
  std::vector<SectionInfo> data_sections; // Stores data sections

  LoadResult load_segments();
  LoadResult parse_sections();
};

} // namespace srrarch

#endif // ELF_LOADER_H