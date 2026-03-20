/**
 * @file elf_loader.cpp
 * @brief Implementation of ELF file loader
 *
 * Implements ELF parsing logic including validation of ELF headers,
 * extraction of program segments, and collection of executable sections.
 * Uses mmap for efficient file access.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "elf_loader.h"
#include "logger.h"
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace srrarch {

const char *load_result_to_string(LoadResult result) {
  switch (result) {
  case LoadResult::SUCCESS:
    return "Success";
  case LoadResult::FILE_NOT_FOUND:
    return "File not found";
  case LoadResult::STAT_FAILED:
    return "Failed to get file stats";
  case LoadResult::INVALID_FILE_SIZE:
    return "Invalid file size (negative or zero)";
  case LoadResult::MMAP_FAILED:
    return "Failed to memory-map file";
  case LoadResult::INVALID_ELF_MAGIC:
    return "Not a valid ELF file";
  case LoadResult::UNSUPPORTED_ARCH:
    return "Only 64-bit ELF is supported";
  case LoadResult::UNSUPPORTED_ENDIAN:
    return "Only little-endian ELF is supported";
  case LoadResult::NO_SECTIONS:
    return "No section headers found";
  case LoadResult::CORRUPT_SECTION:
    return "Section data out of file bounds";
  case LoadResult::SEGMENT_LOAD_FAILED:
    return "Failed to load program segment";
  default:
    return "Unknown error";
  }
}

ElfLoader::ElfLoader()
    : fd(-1), file_map(nullptr), ehdr(nullptr), entry(nullptr),
      is_loaded(false) {
  std::memset(&st, 0, sizeof(st));
}

ElfLoader::~ElfLoader() {
  if (is_loaded) {
    unload();
  }
}

LoadResult ElfLoader::load(const char *filename) {
  LOG_INFO("Loading ELF file: %s", filename);

  // Open ELF file
  fd = open(filename, O_RDONLY);
  if (fd == -1) {
    LOG_ERROR("Failed to open file: %s", strerror(errno));
    return LoadResult::FILE_NOT_FOUND;
  }

  // Get file size
  if (fstat(fd, &st) == -1) {
    LOG_ERROR("fstat failed: %s", strerror(errno));
    close(fd);
    return LoadResult::STAT_FAILED;
  }

  // Validate file size is non-negative and fits in size_t
  if (st.st_size < 0) {
    LOG_ERROR("Invalid file size (negative)");
    close(fd);
    return LoadResult::INVALID_FILE_SIZE;
  }

  LOG_DBG("File size: %ld bytes", st.st_size);

  // Cast to size_t - safe because we checked st.st_size >= 0
  size_t file_size = static_cast<size_t>(st.st_size);

  // Map the entire file into memory
  file_map = static_cast<uint8_t *>(
      mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0));
  if (file_map == MAP_FAILED) {
    LOG_ERROR("mmap failed: %s", strerror(errno));
    close(fd);
    return LoadResult::MMAP_FAILED;
  }

  // Close file descriptor – not needed after mmap
  close(fd);
  fd = -1;
  LOG_DBG("File mapped at %p", file_map);

  // Point to ELF header
  ehdr = reinterpret_cast<Elf64_Ehdr *>(file_map);

  // Basic ELF validation
  if (std::memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
    LOG_ERROR("Not a valid ELF file");
    munmap(file_map, file_size);
    file_map = nullptr;
    return LoadResult::INVALID_ELF_MAGIC;
  }

  // For now, only 64-bit little-endian executables
  if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
    LOG_ERROR("Only 64-bit ELF is supported");
    munmap(file_map, file_size);
    file_map = nullptr;
    return LoadResult::UNSUPPORTED_ARCH;
  }

  if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
    LOG_ERROR("Only little-endian ELF is supported");
    munmap(file_map, file_size);
    file_map = nullptr;
    return LoadResult::UNSUPPORTED_ENDIAN;
  }

  LOG_DBG("ELF header validated: %d program headers, %d section headers",
          ehdr->e_phnum, ehdr->e_shnum);

  // Parse and load segments (existing placeholder)
  LoadResult result = load_segments();
  if (result != LoadResult::SUCCESS) {
    munmap(file_map, file_size);
    file_map = nullptr;
    return result;
  }

  // Parse section headers to collect executable sections
  result = parse_sections();
  if (result != LoadResult::SUCCESS) {
    munmap(file_map, file_size);
    file_map = nullptr;
    return result;
  }

  // Parse symbols
  result = parse_symbols();
  if (result != LoadResult::SUCCESS) {
    munmap(file_map, file_size);
    file_map = nullptr;
    return result;
  }

  // Set entry point
  entry = reinterpret_cast<void *>(ehdr->e_entry);
  LOG_INFO("Entry point: 0x%lx", reinterpret_cast<uintptr_t>(entry));

  is_loaded = true;
  return LoadResult::SUCCESS;
}

LoadResult ElfLoader::load_segments() {
  Elf64_Phdr *phdr = reinterpret_cast<Elf64_Phdr *>(file_map + ehdr->e_phoff);
  for (int i = 0; i < ehdr->e_phnum; ++i) {
    if (phdr[i].p_type == PT_LOAD) {
      LOG_INFO("Loading segment at vaddr 0x%lx, size 0x%lx", phdr[i].p_vaddr,
               phdr[i].p_memsz);
      // TODO: Actual loading logic
    }
  }
  return LoadResult::SUCCESS;
}

LoadResult ElfLoader::parse_sections() {
  exec_sections.clear();
  data_sections.clear();

  if (ehdr->e_shnum == 0 || ehdr->e_shoff == 0) {
    LOG_INFO("No section headers found.");
    return LoadResult::NO_SECTIONS;
  }

  // Get section header string table
  Elf64_Shdr *shstrtab_hdr = reinterpret_cast<Elf64_Shdr *>(
      file_map + ehdr->e_shoff + ehdr->e_shentsize * ehdr->e_shstrndx);
  const char *shstrtab =
      reinterpret_cast<const char *>(file_map + shstrtab_hdr->sh_offset);

  // Iterate over section headers
  for (int i = 0; i < ehdr->e_shnum; ++i) {
    Elf64_Shdr *shdr = reinterpret_cast<Elf64_Shdr *>(file_map + ehdr->e_shoff +
                                                      ehdr->e_shentsize * i);

    // Skip sections that aren't allocated in memory
    if (!(shdr->sh_flags & SHF_ALLOC)) {
      continue;
    }

    SectionInfo info;
    info.name =
        (shdr->sh_name != 0) ? std::string(shstrtab + shdr->sh_name) : "";
    info.addr = shdr->sh_addr;
    info.size = shdr->sh_size;
    info.data = nullptr;
    info.is_bss = false;

    // BSS sections have SHT_NOBITS type - no data in file
    if (shdr->sh_type == SHT_NOBITS) {
      // For BSS, we need to zero-initialize memory, not load from file
      info.is_bss = true;
      LOG_DBG("Found BSS section: %s at 0x%lx (size: 0x%lx)", info.name.c_str(),
              info.addr, info.size);
    }

    // Ensure the section data lies within the file
    else if (shdr->sh_offset + shdr->sh_size <=
             static_cast<uint64_t>(st.st_size)) {
      info.data = file_map + shdr->sh_offset;
    } else {
      LOG_WARN("Section %s data out of file bounds", info.name.c_str());
      info.data = nullptr;
      return LoadResult::CORRUPT_SECTION;
    }

    if (shdr->sh_flags & SHF_EXECINSTR) {
      LOG_DBG("Found executable section: %s at 0x%lx (size: 0x%lx)",
              info.name.c_str(), info.addr, info.size);
      exec_sections.push_back(info);
    } else {
      LOG_DBG("Found data section: %s at 0x%lx (size: 0x%lx)",
              info.name.c_str(), info.addr, info.size);
      data_sections.push_back(info);
    }
  }

  LOG_INFO("Found %zu executable sections and %zu data sections",
           exec_sections.size(), data_sections.size());
  return LoadResult::SUCCESS;
}

LoadResult ElfLoader::parse_symbols() {
  printf_undefined = false;

  // Find the symbol table section
  Elf64_Shdr *symtab_hdr = nullptr;
  Elf64_Shdr *strtab_hdr = nullptr;

  for (int i = 0; i < ehdr->e_shnum; ++i) {
    Elf64_Shdr *shdr = reinterpret_cast<Elf64_Shdr *>(file_map + ehdr->e_shoff +
                                                      ehdr->e_shentsize * i);

    if (shdr->sh_type == SHT_SYMTAB) {
      symtab_hdr = shdr;
    } else if (shdr->sh_type == SHT_STRTAB && i != ehdr->e_shstrndx) {
      // This might be the string table for symbols
      // We'll identify it properly later
      if (strtab_hdr == nullptr && shdr->sh_entsize == 0) {
        strtab_hdr = shdr;
      }
    }
  }

  if (!symtab_hdr || !strtab_hdr) {
    LOG_DBG("No symbol table found");
    return LoadResult::SUCCESS; // Not an error, just no symbols
  }

  // Get string table data
  const char *strtab =
      reinterpret_cast<const char *>(file_map + strtab_hdr->sh_offset);

  // Parse symbols
  size_t num_symbols = symtab_hdr->sh_size / symtab_hdr->sh_entsize;
  Elf64_Sym *sym =
      reinterpret_cast<Elf64_Sym *>(file_map + symtab_hdr->sh_offset);

  for (size_t i = 0; i < num_symbols; ++i) {
    if (sym[i].st_name != 0) {
      std::string name = std::string(strtab + sym[i].st_name);

      // ONLY check for printf
      if (name == "printf") {
        printf_undefined = (sym[i].st_shndx == SHN_UNDEF);
        LOG_DBG("printf is %s", printf_undefined ? "undefined" : "defined");
      }
    }
  }

  return LoadResult::SUCCESS;
}

void ElfLoader::unload() {
  LOG_DBG("Unloading ELF");
  if (file_map) {
    size_t file_size = static_cast<size_t>(st.st_size);
    munmap(file_map, file_size);
    file_map = nullptr;
  }
  if (fd != -1) {
    close(fd);
    fd = -1;
  }
  exec_sections.clear();
  is_loaded = false;
  entry = nullptr;
}

void *ElfLoader::get_entry_point() const { return entry; }

} // namespace srrarch