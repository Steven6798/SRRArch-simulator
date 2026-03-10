#include "elf_loader.h"

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

bool ElfLoader::load(const char *filename) {
  // Open ELF file
  fd = open(filename, O_RDONLY);
  if (fd == -1) {
    std::cerr << "Failed to open file: " << strerror(errno) << std::endl;
    return false;
  }

  // Get file size
  if (fstat(fd, &st) == -1) {
    std::cerr << "fstat failed: " << strerror(errno) << std::endl;
    close(fd);
    return false;
  }

  // Map the entire file into memory
  file_map = static_cast<uint8_t *>(
      mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
  if (file_map == MAP_FAILED) {
    std::cerr << "mmap failed: " << strerror(errno) << std::endl;
    close(fd);
    return false;
  }

  // Close file descriptor – not needed after mmap
  close(fd);
  fd = -1;

  // Point to ELF header
  ehdr = reinterpret_cast<Elf64_Ehdr *>(file_map);

  // Basic ELF validation
  if (std::memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
    std::cerr << "Not a valid ELF file" << std::endl;
    munmap(file_map, st.st_size);
    file_map = nullptr;
    return false;
  }

  // For now, only 64-bit little-endian executables
  if (ehdr->e_ident[EI_CLASS] != ELFCLASS64 ||
      ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
    std::cerr << "Only 64-bit little-endian ELF is supported" << std::endl;
    munmap(file_map, st.st_size);
    file_map = nullptr;
    return false;
  }

  // Parse and load segments (existing placeholder)
  if (!load_segments()) {
    munmap(file_map, st.st_size);
    file_map = nullptr;
    return false;
  }

  // Parse section headers to collect executable sections
  parse_sections();

  // Set entry point
  entry = reinterpret_cast<void *>(ehdr->e_entry);

  is_loaded = true;
  return true;
}

void ElfLoader::unload() {
  if (file_map) {
    munmap(file_map, st.st_size);
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

bool ElfLoader::load_segments() {
  // Iterate over program headers and print each LOAD segment (placeholder)
  Elf64_Phdr *phdr = reinterpret_cast<Elf64_Phdr *>(file_map + ehdr->e_phoff);
  for (int i = 0; i < ehdr->e_phnum; ++i) {
    if (phdr[i].p_type == PT_LOAD) {
      std::cout << "Loading segment at vaddr 0x" << std::hex << phdr[i].p_vaddr
                << ", size 0x" << phdr[i].p_memsz << std::dec << std::endl;
      // TODO: Actual loading logic with proper alignment and permissions
    }
  }
  return true;
}

void ElfLoader::parse_sections() {
  exec_sections.clear();

  if (ehdr->e_shnum == 0 || ehdr->e_shoff == 0) {
    std::cout << "No section headers found." << std::endl;
    return;
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

    if (shdr->sh_flags & SHF_EXECINSTR) {
      SectionInfo info;
      info.name =
          (shdr->sh_name != 0) ? std::string(shstrtab + shdr->sh_name) : "";
      info.addr = shdr->sh_addr;
      info.size = shdr->sh_size;
      // Ensure the section data lies within the file
      if (shdr->sh_offset + shdr->sh_size <=
          static_cast<uint64_t>(st.st_size)) {
        info.data = file_map + shdr->sh_offset;
      } else {
        std::cerr << "Warning: section " << info.name
                  << " data out of file bounds" << std::endl;
        info.data = nullptr;
      }
      exec_sections.push_back(info);
    }
  }
}